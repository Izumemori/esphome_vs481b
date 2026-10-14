#include "connected_sensor.h"

namespace esphome
{
namespace vs481b
{
    void ConnectedSensor::setup() {
        this->parent->available_callback.add([this](bool available) {
            this->publish_state(available);
        });

        this->publish_initial_state(this->parent->is_switch_available());
    }
} // namespace vs481b

} // namespace esphome
