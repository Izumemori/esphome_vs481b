#ifndef HPD_SWITCH_H
#define HPD_SWITCH_H

#include "../vs481b.h"
#include "esphome/components/switch/switch.h"

namespace esphome
{
namespace vs481b
{
class VS481B;

class HPDSwitch : public Component, public switch_::Switch {
    public:
        void setup() override;

        void set_parent(vs481b::VS481B* parent) { this->parent = parent; }

        // The switch doesn't report HPD in its read reply, so this is only the last value sent
        bool assumed_state() override { return true; }

    protected:
        void write_state(bool state) override;

        vs481b::VS481B* parent;
};

} // namespace vs841b
} // namespace esphome

#endif