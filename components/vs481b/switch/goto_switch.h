#pragma once

#include "../vs481b.h"
#include "esphome/components/switch/switch.h"

namespace esphome
{
namespace vs481b
{

class GotoSwitch : public Component, public switch_::Switch, public Parented<VS481B> {
    public:
        void setup() override;
        void dump_config() override;

    protected:
        void write_state(bool state) override;
};

} // namespace vs481b
} // namespace esphome
