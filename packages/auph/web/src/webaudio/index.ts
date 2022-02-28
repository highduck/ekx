import {
    audioContextPause,
    audioContextResume,
    closeContext,
    getAudioContextObject,
    getContext,
    getContextState,
    initContext
} from "./Mixer";
import {
    _getVoiceObj,
    _voiceApplyPitch,
    _voiceChangeDestination,
    _voicePrepareBuffer,
    _voiceSetLoop,
    _voiceSetRunning,
    _voiceStartBuffer,
    _voiceStop,
    createVoiceObj,
    voicePool
} from "./Voice";
import {_getBus, _getBusGain, _setBusConnected, busLine, initBusPool, termBusPool} from "./Bus";
import {setError, warn} from "./debug";
import {
    AuphBuffer,
    AuphBus,
    AuphVoice,
    DefaultBus,
    Flag,
    iMask,
    Message,
    Mixer,
    Name,
    Param,
    tMask,
    Type,
    u31,
    Unit
} from "../protocol/interface";
import {
    _buffer_set_callback,
    _bufferDestroy,
    _bufferLoad,
    _bufferMemory,
    _getBufferObj,
    buffers,
    getNextBufferObj
} from "./Buffer";
import {connectAudioNode, disconnectAudioNode, len, resize, setAudioParamValue} from "./common";

export function setup(): void {
    const ctx = initContext();
    if (ctx) {
        initBusPool(ctx);
    }
}

export function shutdown(): void {
    const ctx = getContext();
    if (ctx) {
        termBusPool();
        resize(voicePool, 1);
        resize(buffers, 1);
        closeContext(ctx);
    }
}

export function load(filepath: string, flags: u31): AuphBuffer {
    let handle = getNextBufferObj();
    if (handle) {
        const ctx = getAudioContextObject();
        if (!ctx) {
            setError(Message.NotInitialized);
            return 0;
        }
        const obj = buffers[handle & iMask]!;
        _bufferLoad(obj, ctx, filepath, flags);
    }
    return handle;
}

export function loadMemory(data: Uint8Array, flags: u31): AuphBuffer {
    let handle = getNextBufferObj();
    if (handle) {
        const ctx = getAudioContextObject();
        if (!ctx) {
            setError(Message.NotInitialized);
            return 0;
        }
        const obj = buffers[handle & iMask]!;
        _bufferMemory(obj, ctx, data, flags);
    }
    return handle;
}

export function load_callback(p_callback: u31, p_userdata: u31): AuphBuffer {
    let handle = getNextBufferObj();
    if (handle) {
        const ctx = getAudioContextObject();
        if (!ctx) {
            setError(Message.NotInitialized);
            return 0;
        }
        const obj = buffers[handle & iMask]!;
        _buffer_set_callback(obj, ctx, p_callback, p_userdata);
    }
    return handle;
}

export function unload(name: Name): void {
    const obj = _getBufferObj(name);
    if (obj) {
        stop(name);
        _bufferDestroy(obj);
    }
}

declare global {
    // class LibraryManager {
    //     static library: LibraryManager;
    // }
    //
    // function mergeInto(library: LibraryManager, module: any): void;

    const HEAP32: Int32Array;
    const HEAPU32: Uint32Array;
    const HEAPF32: Float32Array;

    // const Module: any;

    function _auph_read_to_buffer(p_userdata: u31, p_callback: u31): u31;
}

/***
 *
 * @param buffer
 * @param gain
 * @param pan
 * @param rate
 * @param flags
 * @param bus
 */
export function voice(buffer: AuphBuffer,
                      gain: u31,
                      pan: u31,
                      rate: u31,
                      flags: u31,
                      bus: AuphBus): AuphVoice {
    // arguments check debug
    if (flags & ~(Flag.Running | Flag.Loop)) {
        setError(Message.InvalidArguments);
        return 0;
    }
    ///

    const ctx = getAudioContextObject();
    if (!ctx || ctx.state !== "running") {
        setError(Message.InvalidMixerState, ctx?.state);
        return 0;
    }
    const bufferObj = _getBufferObj(buffer);
    if (!bufferObj) {
        setError(Message.BufferNotFound);
        return 0;
    }
    if (!(bufferObj.s & Flag.Loaded)) {
        setError(Message.BufferIsNotLoaded);
        return 0;
    }
    if (!bufferObj.b) {
        setError(Message.BufferNoData);
        return 0;
    }
    const targetNode = _getBusGain(bus ? bus : DefaultBus);
    if (!targetNode) {
        setError(Message.BusNotFound);
        return 0;
    }
    const voice = createVoiceObj(ctx);
    if (voice === 0) {
        setError(Message.Warning_NoFreeVoices);
        return 0;
    }
    const voiceObj = _getVoiceObj(voice)!;

    voiceObj.s = Flag.Active | flags;
    voiceObj.bf = buffer;
    voiceObj.G = gain;
    voiceObj.P = pan;
    voiceObj.R = rate;
    setAudioParamValue(voiceObj.g.gain, gain / Unit);
    setAudioParamValue(voiceObj.p.pan, pan / Unit - 1);

    // TODO: streamed decoding
    //if (bufferObj.s & Flag.Stream) {
    if (bufferObj.s & Flag.Callback) {
        // const _play_next_chunk = () => {
        //     _voicePrepareBuffer(voiceObj, ctx, bufferObj.b as AudioBuffer);
        //     voiceObj.sn!.loop = false;
        //     const ptr = _auph_read_to_buffer(bufferObj._u, bufferObj._f);
        //     (bufferObj.b as AudioBuffer).copyToChannel(HEAPF32.subarray(ptr >>> 2, 8192 + (ptr >>> 2)), 0, 0);
        //     (bufferObj.b as AudioBuffer).copyToChannel(HEAPF32.subarray(8192 + (ptr >>> 2), (2 * 8192) + (ptr >>> 2)), 1, 0);
        //     voiceObj.sn!.onended = _play_next_chunk;
        //     voiceObj.sn!.start();
        // };
        // _play_next_chunk();
        //_voicePrepareBuffer(voiceObj, ctx, bufferObj.b as AudioBuffer);
        if (flags & Flag.Running) {
            // _voiceStartBuffer(voiceObj);
            voiceObj._s = 1;
            //voiceObj.sn!.loop = true;
            const processor = ctx.createScriptProcessor(8192, 0, 2);
            processor.onaudioprocess = (ev) => {
                // The output buffer contains the samples that will be modified and played
                const outputBuffer = ev.outputBuffer;

                // Loop through the output channels (in this case there is only one)
                const ptr = _auph_read_to_buffer(bufferObj._u, bufferObj._f);
                const output0 = outputBuffer.getChannelData(0);
                const output1 = outputBuffer.getChannelData(1);
                output0.set(HEAPF32.subarray(ptr >>> 2, 8192 + (ptr >>> 2)));
                output1.set(HEAPF32.subarray(8192 + (ptr >>> 2), (2 * 8192) + (ptr >>> 2)));
            };
            //disconnectAudioNode(voiceObj.sn!, voiceObj.p);
            // connectAudioNode(voiceObj.sn!, processor);
            connectAudioNode(processor, voiceObj.p);
            //voiceObj.sn!.start();
            voiceObj.pr = processor;
        }
    }
    else {
        _voicePrepareBuffer(voiceObj, ctx, bufferObj.b as AudioBuffer);
        if (flags & Flag.Running) {
            _voiceStartBuffer(voiceObj);
        }
    }

    // maybe we need to set target before `startBuffer()`
    _voiceChangeDestination(voiceObj, targetNode);
    _voiceApplyPitch(voiceObj, rate);
    return voice;
}

export function stop(name: Name): void {
    if (name === 0) {
        return;
    }

    const type = name & tMask;
    if (type === Type.Voice) {
        const obj = _getVoiceObj(name);
        if (obj) {
            _voiceStop(obj);
        }
    } else if (type === Type.Buffer) {
        const obj = _getBufferObj(name);
        if (obj) {
            for (let i = 1; i < len(voicePool); ++i) {
                const v = voicePool[i]!;
                if (v.bf === name) {
                    _voiceStop(v);
                }
            }
        } else {
            setError(Message.BufferNotFound);
        }
    }
}

export function set(name: Name, param: Param, value: u31): void {
    if (name === 0) {
        return;
    }

    if (name === Mixer && (param & Param.Flags) && (param & Flag.Running)) {
        const ctx = getContext();
        if (ctx) {
            if (value && ctx.state === "suspended") {
                audioContextResume(ctx);
            } else if (!value && ctx.state === "running") {
                audioContextPause(ctx);
            }
        }
    }

    const type = name & tMask;
    if (type === Type.Voice) {
        const obj = _getVoiceObj(name);
        if (obj) {
            if (param & Param.Flags) {
                const enabled = value !== 0;
                if (param & Flag.Running) {
                    _voiceSetRunning(obj, enabled);
                } else if (param & Flag.Loop) {
                    _voiceSetLoop(obj, enabled);
                }
            } else {
                if (param === Param.Gain) {
                    if (obj.G !== value) {
                        obj.G = value;
                        setAudioParamValue(obj.g.gain, value / Unit);
                    }
                } else if (param === Param.Pan) {
                    if (obj.P !== value) {
                        obj.P = value;
                        setAudioParamValue(obj.p.pan, value / Unit - 1);
                    }
                } else if (param === Param.Rate) {
                    if (obj.R !== value) {
                        obj.R = value;
                        _voiceApplyPitch(obj, value);
                    }
                } else if (param === Param.CurrentTime) {
                    // TODO:
                }
            }
        }
    } else if (type === Type.Bus) {
        const obj = _getBus(name);
        if (obj) {
            if (param & Param.Flags) {
                if (param & Flag.Running) {
                    _setBusConnected(obj, !!value);
                }
            } else {
                if (param === Param.Gain) {
                    if (obj.G !== value) {
                        setAudioParamValue(obj.g.gain, value / Unit);
                        obj.G = value;
                    }
                } else {

                }
            }
        }
    }
}

export function get(name: Name, param: u31): u31 {
    let result = 0;

    if (name === Mixer) {
        const ctx = getAudioContextObject();
        if (ctx) {
            if (param === Param.State) {
                result = getContextState(ctx);
            } else if (param === Param.SampleRate) {
                result = ctx.sampleRate | 0;
            }
        }
        return result;
    }

    const type = name & tMask;
    if ((param & Param.Count) && !(name & iMask)) {
        const stateMask = param & Param.StateMask;
        if (type === Type.Voice) {
            result = _countObjectsWithFlags(voicePool, stateMask);
        } else if (type === Type.Bus) {
            result = _countObjectsWithFlags(busLine, stateMask);
        } else if (type === Type.Buffer) {
            result = _countObjectsWithFlags(buffers, stateMask);
        }
        return result;
    }

    if (type === Type.Voice) {
        const obj = _getVoiceObj(name);
        if (obj) {
            if (param === Param.State) {
                result = obj.s;
            } else if (param === Param.Gain) {
                result = obj.G;
            } else if (param === Param.Pan) {
                result = obj.P;
            } else if (param === Param.Rate) {
                result = obj.R;
            } else if (param === Param.Duration) {
                if (obj.sn && obj.sn.buffer) {
                    result = (obj.sn.buffer.duration * Unit) | 0;
                }
            } else if (param === Param.CurrentTime) {
                if (obj.sn && obj.sn.buffer) {
                    // TODO: :(
                }
            } else {
                warn(Message.NotSupported);
            }
        }
        return result;
    }

    if (type === Type.Bus) {
        const obj = _getBus(name);
        if (obj) {
            if (param === Param.State) {
                result = obj.s;
            } else if (param === Param.Gain) {
                result = obj.G;
            } else {
                warn(Message.NotSupported);
            }
        }
        return result;
    }

    if (type === Type.Buffer) {
        const obj = _getBufferObj(name);
        if (obj) {
            if (param === Param.State) {
                result = obj.s;
            } else if (param === Param.Duration) {
                if (obj.b) {
                    result = ((obj.b as AudioBuffer).duration * Unit) | 0;
                }
            } else {
                warn(Message.NotSupported);
            }
        }
    }
    return result;
}

export function vibrate(durationMillis: u31): u31 {
    try {
        if (navigator.vibrate) {
            return navigator.vibrate(durationMillis) ? 0 : 1;
        }
    } catch {
    }
    return 1;
}

/** private helpers **/
function _countObjectsWithFlags(arr: ({ s: u31 } | null)[], mask: u31): u31 {
    let cnt = 0;
    const size = len(arr);
    for (let i = 1; i < size; ++i) {
        const obj = arr[i];
        if (obj && (obj.s & mask) === mask) {
            ++cnt;
        }
    }
    return cnt;
}