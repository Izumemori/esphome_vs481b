#ifndef CONNECTED_SENSOR_H
#define CONNECTED_SENSOR_H

#include "../vs481b.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome
{
namespace vs481b
{
class VS481B;

// Whether the HDMI switch is answering. ESPHome can't mark individual entities as
// unavailable in Home Assistant, so this is the signal to use instead.
class ConnectedSensor : public Component, public binary_sensor::BinarySensor {
    public:
        void setup() override;

        void set_parent(vs481b::VS481B* parent) { this->parent = parent; }

    protected:
        vs481b::VS481B* parent;
};

} // namespace vs481b
} // namespace esphome


#endif
