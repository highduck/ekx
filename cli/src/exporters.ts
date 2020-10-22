import {export_web} from "./web";
import {export_android} from "./android";
import {export_ios} from "./ios";
import {ekc_export_assets, ekc_export_market} from "./assets";
import {Project} from "./project";

const exporters = {
    web: export_web,
    android: export_android,
    ios: export_ios,
    market: (project:Project) => {
        ekc_export_market(project, "gen", "export/market");
    },
    assets: (project:Project) => {
        ekc_export_assets(project);
    }
};

export function addExportBuildStep(project:Project) {
    const exporter = exporters[project.current_target];
    if (exporter) {
        project.build_steps.push(() => exporter(project));

        // project.build_steps.push(() => {
        //     const roots = get_source_roots(project);
        //     console.log(roots);
        // });
    }
}


