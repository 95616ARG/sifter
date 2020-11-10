var base = "http://127.0.0.1:8001"

var friends = new Map();
var chunk_containers = new Map();
function load_structure(structure) {
    console.log(structure);
    var documents = structure["documents"];
    for (var i = 0; i < documents.length; i++) {
        var document_container = $("<div>").appendTo($("body"));
        console.log(documents[i]);
        var chunks = documents[i].chunks;
        for (var j = 0; j < chunks.length; j++) {
            var chunk_container = $("<span>").appendTo(document_container);
            var text = documents[i].text.substr(chunks[j][1], chunks[j][2]);
            chunk_container.text(text);
            if (chunks[j][0] != false) {
                chunk_container.addClass("chunk-in-structure");
                chunk_container.attr("data-chunk-gid", chunks[j][0]);
                chunk_containers.set(chunks[j][0], chunk_container);
            } else {
                chunk_container.attr("data-chunk-gid", "");
            }
        }
        document_container.addClass("document");
        if (documents[i].generated) {
            document_container.addClass("generated");
        }
        var dragger = new PlainDraggable(document_container.get()[0]);
    }
    var maps = structure["maps"];
    for (var i = 0; i < maps.length; i++) {
        var map = maps[i];
        for (var j = 0; j < map.length; j++) {
            if (!friends.has(map[j])) {
                friends.set(map[j], []);
            }
            chunk_containers.get(map[j]).addClass("chunk-in-map");
            for (var k = 0; k < map.length; k++) {
                friends.get(map[j]).push(map[k]);
            }
        }
    }
    console.log(chunk_containers);
}

$("body").on("mouseover", ".chunk-in-structure", function() {
    if ($(this).attr("data-chunk-gid") == "") {
        return;
    }
    var my_friends = friends.get($(this).attr("data-chunk-gid"));
    if (my_friends === undefined) {
        return;
    }
    for (var i = 0; i < my_friends.length; i++) {
        chunk_containers.get(my_friends[i]).addClass("highlight");
    }
});

$("body").on("mouseout", ".chunk-in-structure", function() {
    if ($(this).attr("data-chunk-gid") == "") {
        return;
    }
    var my_friends = friends.get($(this).attr("data-chunk-gid"));
    if (my_friends === undefined) {
        return;
    }
    for (var i = 0; i < my_friends.length; i++) {
        chunk_containers.get(my_friends[i]).removeClass("highlight");
    }
});

$.get(base + "/Structure", function (data) {
    load_structure(data);
});
