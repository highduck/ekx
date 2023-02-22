import {XmlDocument} from "xmldoc";
import {writeTextFileSync} from "../utils/utils.js";

export function writeStringsXML(filepath: string, strings: Record<string, string | undefined>): void {
    const doc = new XmlDocument(`<resources></resources>`);
    for (const key of Object.keys(strings)) {
        const val = strings[key];
        if (val !== undefined) {
            doc.children.push(new XmlDocument(
                `<string name="${key}" translatable="false">${val}</string>`
            ));
        }
    }
    writeTextFileSync(filepath, doc.toString());
}

export function writeColorsXML(filepath: string, colors: { [key: string]: string }): void {
    const doc = new XmlDocument(`<resources></resources>`);
    for (const key of Object.keys(colors)) {
        const val = colors[key];
        doc.children.push(new XmlDocument(
            `<color name="${key}">${val}</color>`
        ));
    }
    writeTextFileSync(filepath, doc.toString());
}