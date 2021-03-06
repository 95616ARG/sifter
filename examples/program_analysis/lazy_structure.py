"""Helper methods for encoding text documents in Triplet Structures.

Here I will use:
    1. 'Word' to refer to an *abstract word*, such a 'hello'.
    2. 'Chunk' to refer to an instance of a word in a document.
    3. 'Node' or 'Symbol' refers to a node in the structure.

Currently only implementing 'flat' reads (i.e., no ASTs), but the goal is to
have the interface simple enough to support ASTs in a straight-forward way.
"""
from ts_lib import TripletStructure
from runtime.runtime import TSRuntime

SPECIAL_CHARACTERS = [
    "(", ")", "[", "]", "{", "}", ".", ";", "*", "/", "+",
    "&", '"', ",", "`", "\n",
]

class LazyStructure:
    """Structure representing multiple LazyDocuments.
    """
    def __init__(self, documents, codelet):
        """Initialize a LazyStructure given a collection of LazyDocuments.

        @codelet(ts) is a callback that should initialize the TripletStructure.
        """
        self.ts = TripletStructure()

        self.documents = documents
        self.document_scopes = dict({
            document: self.ts.scope(f"/:Documents:{i}")
            for i, document in enumerate(documents)
        })

        # Set-union trick from: https://stackoverflow.com/questions/30773911
        self.words = sorted(set().union(*(doc.words for doc in documents)))
        # Maps words to symbol names in the structure.
        self.dictionary = dict({
            word: self.ts[f"/:Dictionary:{i}"]
            for i, word in enumerate(self.words)
        })

        for symbol in self.dictionary.values():
            self.ts[":IsWordMap"].map({symbol: self.ts["/:Word"]})

        for scope in self.document_scopes.values():
            scope[":_IsMember"].map({scope[":_"]: self.ts["/:Document"]})

        self.ts.add_node("/:Chunk")

        for document in self.documents:
            document.InitializeWorkspace(self.ts)

        codelet(self.ts)
        self.rt = TSRuntime(self.ts)

    def ChunkToNode(self, document, chunk):
        """Adds a chunk explicitly into the workspace, if it's not already."""
        assert chunk in document.chunks
        scope = self.document_scopes[document]
        chunk_node = document.ChunkToNode(chunk, self, scope)
        if chunk_node is not None:
            scope[":_IsMember"].map({chunk_node: self.ts["/:Chunk"]})
            self.ts.commit(False)

    def NodeOfChunk(self, document, chunk):
        """Find the node in the workspace corresponding to a chunk."""
        scope = self.document_scopes[document]
        return document.NodeOfChunk(scope, chunk)

    def GetGeneratedDocument(self, index):
        """Parses a document (created by Sifter) out of the workspace.

        Specifically, it updates self.documents[index] to point to a
        LazyGeneratedTextDocument that describes the textual contents parsed
        from the workspace. Used after Sifter completes an analogy to get the
        corresponding code.
        """
        old_document = self.documents[index]
        document = LazyGeneratedTextDocument(self, index)
        self.documents[index] = document
        self.document_scopes[document] = self.document_scopes[old_document]
        self.document_scopes.pop(old_document)

    def AnnotateDocuments(self, fact_map):
        """Annotates the document.

        @fact_map should be a map dict({doc_index: node}).
        """
        fact_node = self.ts["/:DocumentAnnotations:??"]
        for doc_id, annotation in fact_map.items():
            fact_node.map({
                self.ts[f"/:Documents:{doc_id}:_"]: self.ts[annotation],
            })

    def MarkDocumentGenerated(self, index):
        """Indicates that the @index document should be generated by Sifter."""
        self.ts[f"/:DocumentAnnotations:??"].map({
            self.ts[f"/:Documents:{index}:_"]: self.ts["/:Mapper:TOP"],
            self.ts[f"/:Documents:{index}:_IsMember"]: self.ts["/:Mapper:TOP"],
        })

class LazyTextDocument:
    """Represents a single text document."""
    def __init__(self, text, special=None):
        """Initializes the LazyTextDocument, including tokenization.

        @special can contain a list of document-specific tokens.
        """
        self.text = text
        # [(start, length)]
        self.chunks = self.ChunkText(text, special)
        self.words = set(map(self.ChunkWord, self.chunks))
        self.annotations = []

    def AnnotateChunks(self, fact_map):
        """Annotates the document.

        - @fact_map should be a map dict({chunk: node}).
        Each node referenced in the map will be created when the structure is
        initialized. The corresponding fact node will be created at runtime
        when the first referenced chunk is created. Only supports one type per
        concrete.
        """
        self.annotations.append(fact_map)

    def InitializeWorkspace(self, ts):
        """Add an initial set of facts to the workspace."""
        for node in set({"/:Follower:Before", "/:Follower:After"}):
            ts.add_node(node)
        self.annotations = [
            dict({key: ts[value] for key, value in fact_map.items()})
            for fact_map in self.annotations]

    def ChunkToNode(self, chunk, structure, scope):
        """Returns a delta adding @chunk to @structure.ts.

        Also returns the NodeWrapper corresponding to the chunk.
        """
        ts = structure.ts
        chunk_start, _ = chunk
        local_name = f":Chunks:{chunk_start}"
        if local_name in scope:
            return None

        # (1) Add a node for the chunk.
        chunk_node = scope[local_name]
        # (2) Assert that it is an instance of the corresponding word.
        word = structure.dictionary[self.ChunkWord(chunk)]
        scope[f":IsWord:{chunk_start}"].map({chunk_node: word})
        # (3) If the immediately-prior or immediately-following chunk is
        # already in the structure, connect it. TODO(masotoud): Refactor this,
        # also maybe add partial facts anyways?.
        chunk_index = self.chunks.index(chunk)
        if chunk_index > 0:
            left_start, _ = self.chunks[chunk_index - 1]
            left_node = scope.protected()[f":Chunks:{left_start}"]
            if ts.has_node(left_node):
                scope[f":Following:{left_start}:{chunk_start}"].map({
                    ts[left_node]: ts["/:Follower:Before"],
                    chunk_node: ts["/:Follower:After"],
                })
        if (chunk_index + 1) < len(self.chunks):
            right_start, _ = self.chunks[chunk_index + 1]
            right_node = scope.protected()[f":Chunks:{right_start}"]
            if ts.has_node(right_node):
                scope[f":Following:{right_start}:{chunk_start}"].map({
                    chunk_node: ts["/:Follower:Before"],
                    ts[right_node]: ts["/:Follower:After"],
                })
        # (4) If the chunk is annotated, include its annotation.
        for i, fact_map in enumerate(self.annotations):
            if chunk in fact_map:
                scope[f":Annotations:{i}"].map({chunk_node: fact_map[chunk]})

        return chunk_node

    @staticmethod
    def ChunkText(text, special):
        """Tokenizes @text based on standard delimiters and @special.

        Returns a list of (start_index, length) pairs. For example,
        ChunkText("Hi 5+-2", ["-2"])
        = [(0, 2), (2, 1), (3, 1), (4, 1), (5, 2)]
        """
        chunks = []
        # Read characters from @text until either a space is reached or a
        # special word.
        start_chunk = None
        i = 0
        def maybe_wrap_chunk():
            if start_chunk is not None:
                chunks.append((start_chunk, i - start_chunk))
            return None

        while i < len(text):
            if text[i] in (' ', '\n'):
                start_chunk = maybe_wrap_chunk()
                i += 1
            else:
                try:
                    word = next(word for word in special
                                if text[i:].startswith(word))
                    # If there was an existing chunk we were reading, wrap it
                    # up.
                    start_chunk = maybe_wrap_chunk()
                    # Then this word forms a new chunk.
                    chunks.append((i, len(word)))
                    i += len(word)
                except StopIteration:
                    if start_chunk is None:
                        start_chunk = i
                    i += 1
        start_chunk = maybe_wrap_chunk
        return chunks

    def ChunkWord(self, chunk):
        """Returns the string corresponding to @chunk=(start_index, length)."""
        start, length = chunk
        return self.text[start:(start + length)]

    def NodeOfChunk(self, scope, chunk):
        """Returns the workspace node corresponding to @chunk."""
        start, _ = chunk
        return scope.protected()[f":Chunks:{start}"]

    def FindChunk(self, chunk_word):
        """Returns the first chunk with its string being @ChunkWord."""
        for chunk in self.chunks:
            if self.ChunkWord(chunk) == chunk_word:
                return chunk
        return None

    def FindChunks(self, chunk_word):
        """Returns all chunks with corresponding string being @ChunkWord."""
        return [chunk for chunk in self.chunks
                if self.ChunkWord(chunk) == chunk_word]

class LazyGeneratedTextDocument(LazyTextDocument):
    """Like LazyTextDocument, except the contents are read from the workspace.

    The basic use for this is when Sifter _completes_ an analogy, and so
    creates new code in the workspace. A LazyGeneratedTextDocument can read
    that generated code out of the workspace.
    """
    def __init__(self, structure, index):
        """Initialize the LazyGeneratedTextDocument."""
        self.structure = structure
        self.scope = structure.ts.scope(f"/:Documents:{index}", protect=True)
        self.ExtractChunks()

    def ExtractChunks(self):
        """Parses the nodes in the workspace into a string representation."""
        ts, scope = self.structure.ts, self.scope
        chunks = set(
            fact[1]
            for fact in ts.lookup(scope[":_IsMember"], None, "/:Chunk"))
        # (1) Get the poset of the nodes in this document.
        orders = set()
        for fact in ts.lookup(None, None, "/:Follower:Before"):
            map_node, before_node, _ = fact
            if before_node not in chunks:
                continue
            for fact in ts.lookup(map_node, None, "/:Follower:After"):
                after_node = fact[1]
                if after_node in chunks:
                    orders.add((before_node, after_node))
        # (2) Extend the poset into a toset.
        sorted_chunks = []
        while chunks:
            sorted_chunks.append([
                chunk for chunk in sorted(chunks)
                if not any(order[1] == chunk for order in orders)])
            if not sorted_chunks[-1]:
                sorted_chunks[-1] = sorted(chunks) # Cut loops.
            chunks = chunks - set(sorted_chunks[-1])
            orders = set(order for order in orders if set(order) <= chunks)
        linear_chunks = []
        for chunk_layer in sorted_chunks:
            linear_chunks.extend(chunk_layer)
        # (3) Translate the chunks to words.
        chunk_words = []
        for chunk in linear_chunks:
            for word, node in self.structure.dictionary.items():
                if ts.lookup(None, chunk, node.full_name):
                    chunk_words.append(word)
                    break
            else:
                chunk_words.append(f"[Chunk: {chunk}]")
        for i, chunk_word in enumerate(chunk_words):
            if chunk_word[-1] in (";", "{", "}"):
                chunk_words[i] += "\n"
            chunk_words[i] = chunk_words[i] + " "
        self.text = ""
        self.chunks = []
        for word in chunk_words:
            self.chunks.append((len(self.text), len(word)))
            self.text += word
        self.chunk_to_node = dict(zip(self.chunks, linear_chunks))

    def ChunkToNode(self, chunk, structure, scope):
        raise NotImplementedError

    def NodeOfChunk(self, scope, chunk):
        return self.chunk_to_node[chunk]
