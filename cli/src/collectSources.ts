import {search_files} from "./utils";
import * as path from "path";
import {Project} from "./project";

export function collect_source_files(search_path:string, extensions:string[], out_list:string[] = []) {
    for (const ext of extensions) {
        search_files("**/*." + ext, search_path, out_list);
    }
    return out_list;
}

// src_kind - "cpp", "java", "js", etc..
export function collect_src_roots(data, src_kind:string) {
    let result = [];
    if (data && data[src_kind]) {
        result = result.concat(data[src_kind]);
    }
    return result;
}

// rel_to - optional, for example "." relative to current working directory
export function collect_src_roots_all(ctx:Project, src_kind:string, extra_target:string, rel_to:string) {
    let result = [];
    for (const data of ctx.modules) {
        result = result.concat(collect_src_roots(data, src_kind));
        if (extra_target) {
            result = result.concat(collect_src_roots(data[extra_target], src_kind));
        }
    }
    if (rel_to) {
        result = result.map((p) => path.relative(rel_to, p));
    }
    return result;
}

//
//
// function add_roots(to_roots, from_data) {
//     for (const src_type of ["cpp", "java", "js"]) {
//         const from_data_src = from_data[src_type];
//         if (from_data_src) {
//             to_roots[src_type] = (to_roots[src_type] || []).concat(from_data_src);
//         }
//     }
// }
//
// function get_source_roots(ctx) {
//     const roots = {
//         "cpp": [],
//         "java": [],
//         "js": []
//     };
//     const target = ctx.current_target;
//     for (const module_info of ctx.modules) {
//         console.log(module_info.name);
//         add_roots(roots, module_info);
//         if (module_info[target]) {
//             add_roots(roots, module_info[target]);
//         }
//     }
//     return roots;
// }

