import {Message} from "../protocol/interface";

let lastStatus = 0;

export function log(message: string | Message) {
    if (process.env.NODE_ENV !== "production") {
        console.log("auph", message);
    }
}

export function warn(message: string | Message) {
    if (process.env.NODE_ENV !== "production") {
        console.warn("auph", message);
    }
}

export function error(message: string | Message, reason?: any) {
    if (process.env.NODE_ENV !== "production") {
        console.error("auph", message, reason);
    }
}

export function setError(status: Message, context?: any) {
    if (process.env.NODE_ENV !== "production") {
        error(status, context);
    }
    lastStatus = status;
}