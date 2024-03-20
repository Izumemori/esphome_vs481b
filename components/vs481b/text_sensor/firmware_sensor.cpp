#include "firmware_sensor.h"

namespace esphome
{
namespace vs481b
{
    void FirmwareSensor::setup() {
        this->parent->fw_callback.add([this](std::string state) {
            this->publish_state(state);
        });
    }
} // namespace vs481b

} // namespace esphome
