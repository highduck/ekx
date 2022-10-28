import colors from "colors";

const con = console;

type LoggerWrapper = { _diag: boolean } & typeof con;

const a: LoggerWrapper = Object.assign({_diag: false}, con) as LoggerWrapper;

a.debug = (message?: any, ...args: any[]) => {
    if (a._diag) {
        if (message !== undefined && typeof message === "string") {
            message = colors.gray(message);
        }
        con.debug("🐞 " + message, ...args);
    }
};

a.log = (message?: any, ...args: any[]) => {
    if (a._diag) {
        if (message !== undefined && typeof message === "string") {
            message = colors.white.dim(message);
        }
        con.log("🔎 " + message, ...args);
    }
};

a.info = (message?: any, ...args: any[]) => {
    if (message !== undefined && typeof message === "string") {
        // 💬 / ℹ / 🔹
        message = "💬 " + colors.white(message);
    }
    con.info(message, ...args);
};

a.warn = (message?: any, ...args: any[]) => {
    if (message !== undefined && typeof message === "string") {
        message = "🔶 " + colors.yellow(message);
    }
    con.warn(message, ...args);
};

a.error = (message?: any, ...args: any[]) => {
    if (message !== undefined && typeof message === "string") {
        message = "⛔  " + colors.red(message);
    }
    con.error(message, ...args);
};

a.assert = (cond?: any, message?: string, ...options: any[]) => {
    con.assert(cond, message, ...options);
}

export const logger = a;
