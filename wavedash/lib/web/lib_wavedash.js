// https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html

var LibWavedash = {

    $WavedashJs: {
        eventCallback: null,
    },

    WavedashJs_Init: function(eventCallback) {
        console.log("WavedashJs_Init");
        WavedashJs.eventCallback = eventCallback;
        window.Wavedash.init();

        // subscribe to all events
        for (var key in window.Wavedash.Events) {
            if (window.Wavedash.Events.hasOwnProperty(key)) {
                const event = window.Wavedash.Events[key];
                console.log("Registering Wavedash event " + window.Wavedash.Events[key]);
                window.Wavedash.on(event, (payload) => {
                    console.log("on", event, payload);
                    var event_c = stringToNewUTF8(event);
                    var payload_json = JSON.stringify(payload);
                    var payload_json_c = stringToNewUTF8(payload_json);
                    var payload_json_len = lengthBytesUTF8(payload_json);
                    {{{ makeDynCall("viii", "WavedashJs.eventCallback")}}}(event_c, payload_json_c, payload_json_len);
                    Module._free(payload_json_c);
                    Module._free(event_c);
                });
            }
        }
    },

    WavedashJs_CreateLobby: function(visibility, maxPlayers) {
        console.log("WavedashJs_CreateLobby");
        const p = window.Wavedash.createLobby(visibility, maxPlayers);
        p.then(
            function(response) {
                console.log("OK");
                console.log(response);
            },
            function(err) {
                console.log("ERR");
                console.log(err);
            }
        );
    },

    WavedashJs_JoinLobby: function(lobbyId) {
        console.log("WavedashJs_JoinLobby");
        const p = window.Wavedash.joinLobby(lobbyId);
        p.then(
            function(response) {
                console.log("OK");
                console.log(response);
            },
            function(err) {
                console.log("ERR");
                console.log(err);
            }
        );
    }
}

autoAddDeps(LibWavedash, '$WavedashJs');
addToLibrary(LibWavedash);
