#include "connected_sensor.h"

namespace esphome {
namespace vs481b {
static const char *const TAG = "vs481b.connected_sensor";

void ConnectedSensor::setup() {
  this->parent_->available_callback.add([this](bool available) { this->publish_state(available); });

  this->publish_initial_state(this->parent_->is_switch_available());
}

void ConnectedSensor::dump_config() { LOG_BINARY_SENSOR("", "VS481B Connected Sensor", this); }
}  // namespace vs481b

}  // namespace esphome
