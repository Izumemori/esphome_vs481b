#pragma once

#include "../vs481b.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome
{
namespace vs481b
{

// Whether the HDMI switch is answering. ESPHome can't mark individual entities as
// unavailable in Home Assistant, so this is the signal to use instead.
class ConnectedSensor : public Component, public binary_sensor::BinarySensor, public Parented<VS481B> {
    public:
        void setup() override;
        void dump_config() override;
};

} // namespace vs481b
} // namespace esphome
