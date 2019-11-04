#include "project_config.h"
#include "template_vars.h"

namespace ekc {

void fill_template_vars(const project_config_t& config, template_vars_t& vars) {
    auto& data = vars.data;
    data["name"] = config.name;
    data["title"] = config.title;
    data["desc"] = config.desc;
    data["version_name"] = config.version_name;
    data["version_code"] = config.version_code;
    data["build_number"] = config.build_number;
    data["binary_name"] = config.binary_name;
    data["pwa_url"] = config.pwa_url;

    template_vars_t::data_type html{};
    if (!config.html.google_analytics_property_id.empty()) {
        html["google_analytics_property_id"] = config.html.google_analytics_property_id;
    }
    html["background_color"] = config.html.background_color;
    html["text_color"] = config.html.text_color;
    html["url"] = config.html.url;
    html["image_url"] = config.html.image_url;

    data.set("html", html);
}

}