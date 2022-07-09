// if you modify seed value, you should also modify it in `@ekx/std` package: `ek/hash.h`
const seed = 0x811C9DC5;

// FNV-1a hash
export function H(str: string): number {
    let hash = seed;
    // while we use ASCII for identifiers, we are OK. Then it's better to synchronize string encoding between native ad JS env
    for (let i = 0; i < str.length; ++i) {
        hash ^= str.charCodeAt(i);
        hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24)
    }
    return hash;
}
