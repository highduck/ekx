import {Project} from "../../modules/cli/project.ts";

export async function setup(project: Project) {
    project.addModule({
        name: "audio",
        cpp: "src",
        apple: {
            cpp_flags: {
                files: [
                    "src/ek_audio_auph.c"
                ],
                flags: "-x objective-c"
            },
        },
        android: {
            cpp_flags: {
                files: [
                    "src/ek_audio_auph.c"
                ],
                flags: "-xc++ -std=c++17"
            },
        }
    });
    await project.importModule("../auph/ek.ts");
    await project.importModule("../core/ek.ts");
    await project.importModule("../app/ek.ts");
}
