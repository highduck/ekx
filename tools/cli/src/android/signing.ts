import * as path from "path";
import {copyFile, isFile} from "../utils";

interface SigningConfig {
    key_alias: string;
    key_password: string;
    store_keystore: string;
    store_password: string;
}

type SigningConfigs = { [flavor: string]: SigningConfig };

function printSigningConfig(flavor: string, config: SigningConfig) {
    let str = "";
    if (flavor && config) {
        str = `
    ${flavor} {
        storeFile file('${path.basename(config.store_keystore)}')
        storePassword '${config.store_password}'
        keyAlias '${config.key_alias}'
        keyPassword '${config.key_password}'
    }`;
    }
    return str;
}

export function printSigningConfigs(configs: SigningConfigs) {
    let str = "";
    if (configs) {
        for (const flavor of Object.keys(configs)) {
            str += printSigningConfig(flavor, configs[flavor]);
        }
    }
    return str;
}

export function copySigningKeys(configs: SigningConfigs, basePath: string) {
    if (configs) {
        for (const flavor of Object.keys(configs)) {
            const config = configs[flavor];
            if (config && config.store_keystore) {
                const filepath = config.store_keystore;
                let src = filepath;
                if (!isFile(filepath)) {
                    src = path.resolve(basePath, filepath);
                    if (!isFile(src)) {
                        console.warn("missing keystore", filepath);
                        continue;
                    }
                }
                copyFile(src, path.join("app", path.basename(src)));
            }
        }
    }
}