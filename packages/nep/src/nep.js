//const msgpack = require("msgpack-lite");

mergeInto(LibraryManager.library, {
    js_call: function (p_data, size, call_id) {
        const msgpack = window["msgpack"];
        const args = p_data ? msgpack.decode(HEAPU8.subarray(p_data, p_data + size)) : null;
        let calls = window["nep_calls"];
        if(!calls) {
            calls = {};
            window["nep_calls"] = calls;
        }
        const handler = window["nep_handler"];
        calls[call_id] = {
            args: args,
            call_id: call_id,
            set_result: (obj) => {
                const serialized_result = msgpack.encode(obj);
                const p_out = _malloc(serialized_result.byteLength);
                HEAPU8.set(serialized_result, p_out);
                _js_resolve_call(call_id, p_out, serialized_result.byteLength);
                //_free(p_out);
                calls[call_id] = null;
            }
        };
        handler(calls[call_id]);
        return 0;
    }
});