import {xmldoc} from "../deps.ts";
const {XmlDocument} = xmldoc;

export function writeStringsXML(filepath: string, strings: { [key: string]: string }): void {
    const doc = new XmlDocument(`<resources></resources>`);
    for (const key of Object.keys(strings)) {
        const val = strings[key];
        doc.children.push(new XmlDocument(
            `<string name="${key}" translatable="false">${val}</string>`
        ));
    }
    Deno.writeTextFileSync(filepath, doc.toString());
}

export function writeColorsXML(filepath: string, colors:{[key:string]:string}): void {
    const doc = new XmlDocument(`<resources></resources>`);
    for (const key of Object.keys(colors)) {
        const val = colors[key];
        doc.children.push(new XmlDocument(
            `<color name="${key}">${val}</color>`
        ));
    }
    Deno.writeTextFileSync(filepath, doc.toString());
}