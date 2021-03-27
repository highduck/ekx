#pragma once

#include <cstdint>
#include <string>
#include <ek/math/box.hpp>

namespace ek {

// ffmpeg -f gif -i result.gif -movflags faststart -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" result2.mp4
// ffmpeg -framerate 50 -start_number 0 -i "result/frame%d.png" -c:v libx264 -pix_fmt yuv420p out.mp4
//void gifRecordFrame() {
//    const uint32_t framesTotal = 314;
//
//    if (gif == nullptr) {
//        // start recording
//        gif = new screen_recorder("result", rect_u{0, 100, 1024, 1024});
//    }
//
//    gif->render();
//    if (gif->framesRecorded() > framesTotal) {
//        delete gif;
//        // stop recording
//        exit(0);
//    }
//}

class screen_recorder {
public:
    screen_recorder(const std::string& filename, const rect_u& rect);

    ~screen_recorder();

    void render();

    uint32_t recorded_frames_num() const {
        return frame_;
    }

private:
    std::string filename_;
    uint8_t* buffer_ = nullptr;
    uint8_t* buffer_swap_ = nullptr;
    rect_u rect_;
    uint32_t frame_ = 0;
};

}


