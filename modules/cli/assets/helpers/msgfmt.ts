import {mo, po} from "gettext-parser";
import {readFile, writeFile} from "node:fs/promises";

export const msgfmt = async (src: string, dest: string) => {
    const pofile = await readFile(src);
    const messages = po.parse(pofile);
    const buffer = mo.compile(messages);
    await writeFile(dest, buffer);
};