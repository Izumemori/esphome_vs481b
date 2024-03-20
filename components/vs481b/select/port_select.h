#ifndef PORT_SELECT_H
#define PORT_SELECT_H

#include "../vs481b.h"
#include "esphome/components/select/select.h"

namespace esphome
{
namespace vs481b
{
class VS481B;

class PortSelect : public Component, public select::Select {
    public:
        void setup() override;

        void set_parent(vs481b::VS481B* parent) { this->parent = parent; }

    protected:
        void control(const std::string& value) override;
        vs481b::VS481B* parent;
};

} // namespace vs841b
} // namespace esphome


#endif