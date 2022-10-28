import {export_web} from "./web/web.js";
import {export_android} from "./android/android.js";
import {export_ios} from "./ios/ios.js";
import {buildAssetPackAsync} from "./assets.js";
import {buildMarketingAssets} from "./assets/helpers/market.js";
import {Project} from "./project.js";
import {screenshots} from "./screenshots/index.js";
import {updateGoogleServicesConfig} from "./firebase/index.js";
import {buildDev} from "./dev/index.js";

const exporters = {
    web: export_web,
    android: export_android,
    ios: export_ios,
    market: (project: Project) => buildMarketingAssets(project, "export/market"),
    assets: (project: Project) => buildAssetPackAsync(project, "build/assets", true),
    screenshots,
    updateGoogleServicesConfig,
    dev: buildDev
};

export function addExportBuildStep(project: Project) {
    const exporter = (exporters as any)[project.current_target];
    if (exporter) {
        project.build_steps.push(() => exporter(project));

        // project.build_steps.push(() => {
        //     const roots = get_source_roots(project);
        //     logger.log(roots);
        // });
    }
}


