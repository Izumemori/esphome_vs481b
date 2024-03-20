#include "hpd_switch.h"

namespace esphome{
namespace vs481b {
    static const char *TAG = "vs481b.hpd_switch";

    void HPDSwitch::setup() {
        this->parent->hpd_callback.add([this](bool state) {
            this->publish_state(state);
        }) ;
    }

    void HPDSwitch::write_state(bool state) {
        if (!this->parent->set_hpd(state))
            this->publish_state(this->state);
    }
}
}