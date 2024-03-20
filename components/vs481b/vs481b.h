#ifndef VS841B_H
#define VS841B_H

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/macros.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"

#include "select/port_select.h"

#include "mode.h"
#include "settings.h"

namespace esphome {
namespace vs481b {
class VS481B : public uart::UARTDevice, public PollingComponent {
    public:
        inline void register_ports(std::vector<std::string> ports) { this->ports = ports; }
        inline void register_defaults(Settings settings) 
        { 
            this->default_settings = settings;
        }

        void update() override;

        bool set_port(int port);
        bool set_output(bool state);
        bool set_mode(Mode mode);
        bool set_auto_port(int port);
        bool set_goto(bool state);
        bool set_fw(std::string fw);

        bool set_hpd(bool state);

        inline std::vector<std::string> get_ports() {
            return this->ports;
        }

        /*
            Callbacks
        */
        CallbackManager<void(int)> port_callback;
        CallbackManager<void(bool)> output_callback;
        CallbackManager<void(Mode)> mode_callback;
        CallbackManager<void(int)> auto_port_callback;
        CallbackManager<void(bool)> goto_callback;
        CallbackManager<void(std::string)> fw_callback;
        CallbackManager<void(bool)> hpd_callback;

    private:
        std::string read_line();
        bool send_command(std::string command, bool flush = true);
        uint32_t get_time_per_char();

    protected:
        Settings current_settings;
        Settings default_settings;

        bool has_been_setup = false;

        std::vector<std::string> ports;
};
}
}

#endif