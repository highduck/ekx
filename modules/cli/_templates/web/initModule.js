
var Module = Module || {};
var Module0 = Module0 || {};
Module0.loaders = [];
Module0.started = false;
Module0.globalErrorCapture = null;

function binary(e, i) {
    return new Promise((n, r) => {
        var t = new XMLHttpRequest();
        t.open("GET", e, !0);
        t.responseType = "arraybuffer";
        t.onload = () => {
            Module0.loaders[i] = 1.0;
            updateStatus();
            n(t.response);
        };
        t.onprogress = function(pr) {
            if(pr.total > 0) {
                Module0.loaders[i] = pr.loaded / pr.total;
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
        t.type = "module";
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
    Module0.wasm = wasm;
    return import("./{{name}}.js?v={{{version_code}}}").then(factory => {
        updateStatus();
        factory.default({wasm: wasm}).then((m)=> {
            Module = m;
            Module0.started = true;
            updateStatus();
        });
    });
})

function updateStatus() {
    var spinner = document.getElementById('spinner');
    var game = document.getElementById('gameview');
    var status = document.getElementById('status');
    if(game) {
        if(Module0.globalErrorCapture) {
            status.innerHTML = Module0.globalErrorCapture;
            spinner.style.display = 'block';
            game.style.display = 'none';
            return;
        }
        if(Module0.started) {
            if (game.style.display !== 'block') {
                spinner.style.display = 'none';
                game.style.display = 'block';
            }
        } else {
            var progress = ((Module0.loaders[0]|0) + (Module0.loaders[1]|0)) / 2;
            spinner.style.display = 'block';
            game.style.display = 'none';
            status.innerHTML = ((progress * 100) | 0) + "%";
        }
    }
}

function init() {
    updateStatus();
    window.onerror = function (e) {
        Module0.globalErrorCapture = e;
    };
}