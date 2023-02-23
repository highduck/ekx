import {init} from "./npm.js";
import {logger} from "../cli/logger.js";

try {
    init();
}
catch(e) {
    logger.error("Error: ", e);
}