import {logger} from "../logger";
import * as fs from "fs";
import {executeAsync} from "../utils";
import {Project} from "../project";

function getToken(project: Project): string {
    let token = process.env.FIREBASE_TOKEN;
    if (!token && project.web.firebaseToken) {
        try {
            if (fs.existsSync(project.web.firebaseToken)) {
                token = fs.readFileSync(project.web.firebaseToken, 'utf-8');
            } else {
                logger.error(`Firebase Token file path not found`);
            }
        } catch {
            logger.error(`Cannot read Firebase Token`);
        }
    }
    return token;
}

export async function deployFirebaseHosting(project: Project) {
    // always deploy just to the default firebase hosting
    logger.info("Publish Web beta to Firebase host");
    const args = [];
    const token = getToken(project);
    if (token) {
        args.push("--token", token);
    } else {
        logger.warn("No Firebase Token. Trying deploy with local firebase auth");
    }

    await executeAsync("firebase", [
        "deploy",
        "--only", "hosting",
        ...args
    ]);
}