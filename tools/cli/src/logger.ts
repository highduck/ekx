import {Console} from "console";
import * as chalk from "chalk";

//process.stdout.setEncoding("utf8");

const con = new Console({
    stdout: process.stdout,
    stderr: process.stderr,
    inspectOptions: {depth: 0, colors: true, compact: 3}
});

type LoggerWrapper = { _diag: boolean } & typeof con;

const a: LoggerWrapper = Object.assign({_diag: false}, con) as LoggerWrapper;

a.debug = (message?: any, ...args: any[]) => {
    if (a._diag) {
        if (message !== undefined && typeof message === "string") {
            message = chalk.gray(message);
        }
        con.debug("🐞 " + message, ...args);
    }
};

a.log = (message?: any, ...args: any[]) => {
    if (a._diag) {
        if (message !== undefined && typeof message === "string") {
            message = chalk.white(message);
        }
        con.log("🔹 " + message, ...args);
    }
};

a.info = (message?: any, ...args: any[]) => {
    if (message !== undefined && typeof message === "string") {
        //message = "ℹ️ " + chalk.whiteBright(message);
        message = "🔔 " + chalk.whiteBright(message);
    }
    con.info(message, ...args);
};

a.warn = (message?: any, ...args: any[]) => {
    if (message !== undefined && typeof message === "string") {
        message = "🔶 " + chalk.yellowBright(message);
    }
    con.warn(message, ...args);
};

a.error = (message?: any, ...args: any[]) => {
    if (message !== undefined && typeof message === "string") {
        message = "⛔  " + chalk.red(message);
    }
    con.error(message, ...args);
};

a.assert = (cond?: any, message?: string, ...options: any[]) => {
    con.assert(cond, message, ...options);
}

export const logger = a;
