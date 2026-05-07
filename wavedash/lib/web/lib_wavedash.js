// https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html

var LibWavedash = {

    Wavedash: {
    },

    WavedashJs_Init: function(config) {
        window.Wavedash.init(config);
    },
}

autoAddDeps(LibWavedash, 'Wavedash');
addToLibrary(LibWavedash);
