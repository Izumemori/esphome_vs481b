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

    protected:
        void write_state(bool state) override;

        vs481b::VS481B* parent;
};

} // namespace vs841b
} // namespace esphome

#endif