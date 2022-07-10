
var Module = Module || {};
Module.loaders = [];
Module.started = false;
Module.globalErrorCapture = null;

function binary(e, i) {
    return new Promise((n, r) => {
        var t = new XMLHttpRequest();
        t.open("GET", e, !0);
        t.responseType = "arraybuffer";
        t.onload = () => {
            Module.loaders[i] = 1.0;
            updateStatus();
            n(t.response);
        };
        t.onprogress = function(pr) {
            if(pr.total > 0) {
                Module.loaders[i] = pr.loaded / pr.total;
                updateStatus();
            }
        };
        t.send();
    })
}

function script(e) {
    return new Promise((n, r) => {
        var t = document.createElement("script");
        t.src = e;
        t.onload = () => {
            n();
        };
        document.body.appendChild(t);
    })
}

// Promise.all([binary("{{name}}.js?v={{{version_code}}}", 0), binary("{{name}}.wasm?v={{{version_code}}}", 1)]).then(e => {
//     Module.wasm = e[1];
//     var n = URL.createObjectURL(new Blob([e[0]], {type: "application/javascript"}));
//     script(n).then(() => {
//         URL.revokeObjectURL(n);
//         Module.started = true;
//         updateStatus();
//     })
// });

binary("{{name}}.wasm?v={{{version_code}}}").then(wasm => {
    Module.wasm = wasm;
    return script("{{name}}.js?v={{{version_code}}}").then(_ => {
        Module.started = true;
        updateStatus();
    });
})

function updateStatus() {
    var spinner = document.getElementById('spinner');
    var game = document.getElementById('gameview');
    var status = document.getElementById('status');
    if(game) {
        if(Module.globalErrorCapture) {
            status.innerHTML = Module.globalErrorCapture;
            spinner.style.display = 'block';
            game.style.display = 'none';
            return;
        }
        if(Module.started) {
            if (game.style.display !== 'block') {
                spinner.style.display = 'none';
                game.style.display = 'block';
            }
        } else {
            var progress = ((Module.loaders[0]|0) + (Module.loaders[1]|0)) / 2;
            spinner.style.display = 'block';
            game.style.display = 'none';
            status.innerHTML = ((progress * 100) | 0) + "%";
        }
    }
}

function init() {
    updateStatus();
    window.onerror = function (e) {
        Module.globalErrorCapture = e;
    };
}