import {BytesWriter} from "./BytesWriter";

export class StringTable {
    strings: string[] = [""];

    index(str: string): number {
        if (!str || str.length === 0) {
            return 0;
        }
        const strings = this.strings;
        let i = strings.indexOf(str);
        if (i < 0) {
            i = strings.length;
            strings.push(str);
        }
        return i;
    }

    write(output:BytesWriter) {
        // const len = this.strings.length;
        // const stringsOffset = output.size + 4 + (len << 2);
        // output.writeI32(len);
        // for(len)
    }

}