import {logger} from "../logger.ts";
import {fs} from "../../deps.ts";
import {executeAsync} from "../utils.ts";
import {Project} from "../project.ts";

function getToken(project: Project): string|undefined {
    let token = process.env.FIREBASE_TOKEN;
    if (!token && project.web.firebaseToken) {
        try {
            if (fs.existsSync(project.web.firebaseToken)) {
                token = Deno.readTextFileSync(project.web.firebaseToken);
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