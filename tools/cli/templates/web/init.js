var Module = {};

function init() {
    var spinner = document.getElementById('spinner');
    var game = document.getElementById('gameview');
    var status = document.getElementById('status');

    spinner.style.display = 'block';
    game.style.display = 'none';
    status.innerHTML = "Downloading...";

    Module = {
        preRun: [],
        postRun: [],
        print: void 0,
        printErr: function (e) {
        },
        canvas: function () {
            var e = document.getElementById("gameview");
            return e.addEventListener("webglcontextlost", function (e) {
                alert("WebGL context lost. You will need to reload the page.");
                e.preventDefault();
            }, !1);
        }(),
        setStatus: function (e) {
            status.textContent = e;
            if (e === "" && game.style.display !== 'block') {
                spinner.style.display = 'none';
                game.style.display = 'block';
            }
        },
        monitorRunDependencies: function (e) {
            // console.info(e);
        }
    };

    window.onerror = function (e) {
        status.innerHTML = e;
        spinner.style.display = 'block';
        game.style.display = 'none';
    };

    var script = document.createElement("script");
    script.async = true;
    script.src = "{{name}}.js?v={{{version_code}}}";
    document.getElementsByTagName("head")[0].append(script);
}

if (typeof navigator.serviceWorker === "undefined") {
    console.log('Service worker is not supported');
}
else {
    if(navigator.serviceWorker.controller) {
        navigator.serviceWorker.getRegistration().then(function(reg) {
            if(reg) {
                function confirmUpdate(reg) {
                    navigator.serviceWorker.addEventListener('controllerchange', function() {
                        if (this.refreshing) {
                            return;
                        }
                        this.refreshing = true;
                        window.location.reload();
                    });

                    // our new instance is now waiting for activation (its state is 'installed')
                    // we now may invoke our update UX safely

                    // this is for update flow manually
                    if (confirm("Game is updated! Do you want to restart now?")) {
                        reg.waiting.postMessage('SKIP_WAITING');
                    } else {
                        // do nothing
                    }
                }
                if(reg.waiting) {
                    confirmUpdate(reg);
                }
                else {
                    reg.addEventListener('updatefound', function () {
                        reg.installing.addEventListener('statechange', () => {
                            if (reg.waiting) {
                                // our new instance is now waiting for activation (its state is 'installed')
                                // we now may invoke our update UX safely
                                confirmUpdate(reg);
                            } else {
                                // apparently installation must have failed (SW state is 'redundant')
                                // it makes no sense to think about this update any more
                            }
                        });
                    });
                }
            }
        });
    }
    else {
        navigator.serviceWorker.register('sw.js', {
            scope: './'
        }).then(function(reg) {
            if(reg.installing) {
                console.log('Service worker installing');
            } else if(reg.waiting) {
                console.log('Service worker installed');
            } else if(reg.active) {
                console.log('Service worker active');
            }
        }).catch(function(e) {
            // registration failed
            console.log('Registration failed with', e);
        });
    }
}