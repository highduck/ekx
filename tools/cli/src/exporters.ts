import {export_web} from "./web/web";
import {export_android} from "./android/android";
import {export_ios} from "./ios/ios";
import {buildAssetPackAsync} from "./assets";
import {buildMarketingAssets} from "./assets/market";
import {Project} from "./project";
import {screenshots} from "./screenshots";
import {updateGoogleServicesConfig} from "./firebase";
import {buildDev} from "./dev";

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
    const exporter = exporters[project.current_target];
    if (exporter) {
        project.build_steps.push(() => exporter(project));

        // project.build_steps.push(() => {
        //     const roots = get_source_roots(project);
        //     logger.log(roots);
        // });
    }
}


