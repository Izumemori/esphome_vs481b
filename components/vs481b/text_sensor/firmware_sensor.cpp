#include "firmware_sensor.h"

namespace esphome
{
namespace vs481b
{
    static const char *const TAG = "vs481b.firmware_sensor";

    void FirmwareSensor::setup() {
        this->parent_->fw_callback.add([this](const std::string &state) {
            this->publish_state(state);
        });
    }

    void FirmwareSensor::dump_config() {
        LOG_TEXT_SENSOR("", "VS481B Firmware Sensor", this);
    }
} // namespace vs481b

} // namespace esphome
