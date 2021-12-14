import * as fs from "fs";

const conversionBuffer = new ArrayBuffer(8);
const conversionU8 = new Uint8Array(conversionBuffer);
const conversionU32 = new Uint32Array(conversionBuffer);
const conversionI32 = new Int32Array(conversionBuffer);
const conversionF32 = new Float32Array(conversionBuffer);

export class BytesWriter {
    bytes: Uint8Array;
    size: number = 0;

    constructor(private _capacity: number = 128) {
        this.setBytes(new Uint8Array(_capacity));
    }

    private setBytes(bytes: Uint8Array) {
        this.bytes = bytes;
    }

    ensureSize(size: number) {
        if (size > this._capacity) {
            while (size > this._capacity) {
                this._capacity <<= 1;
            }
            const newBytes = new Uint8Array(this._capacity);
            newBytes.set(this.bytes, 0);
            this.setBytes(newBytes);
        }
    }

    pad32() {
        while ((this.size & 0x3) !== 0) {
            this.bytes[this.size++] = 0;
        }
    }

    writeStringAscii(str: string): void {
        const len = str.length;
        let p = this.size;
        this.ensureSize(p + len + 1);
        const bytes = this.bytes;
        for (let i = 0; i < len; ++i) {
            bytes[p++] = str.charCodeAt(i);
        }
        bytes[p++] = 0;
        this.size = p;
    }

    save(filepath: string) {
        fs.writeFileSync(filepath, new Uint8Array(this.bytes.buffer.slice(0, this.size)));
    }

    writeString(s: string) {
        this.ensureSize(this.size + 4 + (s.length << 2) + 1);
        const bytesWritten = this.writeUtf8(s, this.size + 4);
        this.writeU32(bytesWritten);
        this.size += bytesWritten;

        // null-terminator
        this.bytes[this.size++] = 0;
    }

    writeUtf8(s: string, destOffset: number): number {
        let i = 0;
        let p = destOffset;
        const bytes = this.bytes;
        while (i < s.length) {
            let c = s.charCodeAt(i++);
            // surrogate pair
            if (0xD800 <= c && c <= 0xDBFF) {
                c = (c - 0xD7C0 << 10) | (s.charCodeAt(i++) & 0x3FF);
            }

            if (c <= 0x7F) {
                bytes[p++] = c;
            } else if (c <= 0x7FF) {
                bytes[p++] = 0xC0 | (c >> 6);
                bytes[p++] = 0x80 | (c & 63);
            } else if (c <= 0xFFFF) {
                bytes[p++] = 0xE0 | (c >> 12);
                bytes[p++] = 0x80 | ((c >> 6) & 63);
                bytes[p++] = 0x80 | (c & 63);
            } else {
                bytes[p++] = 0xF0 | (c >> 18);
                bytes[p++] = 0x80 | ((c >> 12) & 63);
                bytes[p++] = 0x80 | ((c >> 6) & 63);
                bytes[p++] = 0x80 | (c & 63);
            }
        }
        return p - destOffset;
    }

    writeBytes(inBytes: Uint8Array, size: number) {
        let p = this.size;
        this.ensureSize(p + size);
        const bytes = this.bytes;
        for (let i = 0; i < size; ++i) {
            bytes[p++] = inBytes[i];
        }
        this.size = p;
    }

    writeSection(section: BytesWriter) {
        const sz = section.size;
        this.writeU32(sz);
        this.writeBytes(section.bytes, sz);
    }

    writeU32(v: number) {
        conversionU32[0] = v;
        this.writeBytes(conversionU8, 4);
    }

    writeI32(v: number) {
        conversionI32[0] = v;
        this.writeBytes(conversionU8, 4);
    }

    writeF32(v: number) {
        conversionF32[0] = v;
        this.writeBytes(conversionU8, 4);
    }
}