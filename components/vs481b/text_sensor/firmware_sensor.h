#ifndef FIRMWARE_SENSOR_H
#define FIRMWARE_SENSOR_H

#include "../vs481b.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome
{
namespace vs481b
{
class VS481B;

class FirmwareSensor : public Component, public text_sensor::TextSensor {
    public:
        void setup() override;

        void set_parent(vs481b::VS481B* parent) { this->parent = parent; }

    protected:
        vs481b::VS481B* parent;
};

} // namespace vs841b
} // namespace esphome


#endif