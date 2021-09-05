export function parseBoolean(v?: string): boolean {
    if (v) {
        const lc = v.toLowerCase();
        return lc === "true" || lc === "on" || lc === "1";
    }
    return false;
}

export function readFloat(v:string|undefined|null, defaultValue:number):number {
    if(v) {
        return parseFloat(v);
    }
    return defaultValue;
}