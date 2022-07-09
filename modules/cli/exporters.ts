import {export_web} from "./web/web.ts";
import {export_android} from "./android/android.ts";
import {export_ios} from "./ios/ios.ts";
import {buildAssetPackAsync} from "./assets.ts";
import {buildMarketingAssets} from "./assets/helpers/market.ts";
import {Project} from "./project.ts";
import {screenshots} from "./screenshots/index.ts";
import {updateGoogleServicesConfig} from "./firebase/index.ts";
import {buildDev} from "./dev/index.ts";

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


