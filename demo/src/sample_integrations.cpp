#include <ui/minimal.hpp>
#include <ek/ext/game_center/game_center.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include "sample_integrations.hpp"

namespace ek {

SampleIntegrations::SampleIntegrations() :
        SampleBase() {
    title = "SERVICE";

    // TODO: show all leaderboards, achievements, ads

    float2 pos{360.0f / 2, 100.0f};
    auto btn = createButton("LEADERBOARD", [] {
        leader_board_show("CgkIpvfh798IEAIQAA");
    });
    set_position(btn, pos);
    get_drawable<Text2D>(btn).rect.set(-100, -25, 200, 50);
    append(container, btn);
    pos.y += 100;

    btn = createButton("POST RESULT", [] {
        static int bestScore = 1;
        leader_board_submit("CgkIpvfh798IEAIQAA", ++bestScore);
    });
    set_position(btn, pos);
    get_drawable<Text2D>(btn).rect.set(-100, -25, 200, 50);
    append(container, btn);
}

}