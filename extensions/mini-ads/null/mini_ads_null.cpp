#include <ek/mini_ads.hpp>
#include <platform/application.hpp>
#include <ek/util/timer.hpp>
#include <ek/util/logger.hpp>

namespace ek {

class ads_fake_timer : public application_listener_t {
public:
    ~ads_fake_timer() override = default;

    void on_event(const event_t&) override {}

    void on_draw_frame() override {
        if (fn_ && time_ > 0.0f && timer_.read_seconds() >= time_) {
            time_ = 0.0f;
            fn_();
        }
    }

    void start(std::function<void()> fn, float time) {
        fn_ = std::move(fn);
        timer_.reset();
        time_ = time;
    }

private:
    std::function<void()> fn_;
    float time_ = 0.0f;

    timer_t timer_;
};

// TODO: signals to core module
static std::function<void(AdsEventType type)> ads_registered_callbacks;

void ads_init(const ads_config_t&) {}

void ads_reset_purchase() {}

void ads_set_banner(int) {}

static bool loaded_ = false;
static ads_fake_timer* fake_timer = nullptr;

void ads_play_reward_video() {
    if (!fake_timer) {
        fake_timer = new ads_fake_timer();
        g_app.listen(fake_timer);
    }

    if (ads_registered_callbacks) {
        if (!loaded_) {
            fake_timer->start([] {
                loaded_ = true;
                EK_INFO << "ADS_VIDEO_LOADING";
                ads_registered_callbacks(AdsEventType::ADS_VIDEO_LOADING);
                EK_INFO << "ADS_VIDEO_REWARD_LOADED";
                ads_registered_callbacks(AdsEventType::ADS_VIDEO_REWARD_LOADED);
            }, 5.0f);
        } else {
            fake_timer->start([] {
                loaded_ = false;
                EK_INFO << "ADS VIDEO REWARDED";
                ads_registered_callbacks(AdsEventType::ADS_VIDEO_REWARDED);
            }, 5.0f);

            //ads_registered_callbacks(AdsEventType::ADS_VIDEO_REWARDED);
        }
//#ifdef NDEBUG
//            ads_registered_callbacks(AdsEventType::ADS_VIDEO_REWARD_FAIL);
//#else
//
//
//            ads_registered_callbacks(AdsEventType::ADS_VIDEO_REWARDED);
//#endif
    }
}

void ads_show_interstitial() {}

void ads_listen(const std::function<void(AdsEventType type)>& callback) {
    ads_registered_callbacks = callback;
}

void ads_purchase_remove() {}

void init_billing(const char*) {}

}
