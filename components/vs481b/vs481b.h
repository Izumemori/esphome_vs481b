#ifndef VS841B_H
#define VS841B_H

#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include "mode.h"
#include "settings.h"

namespace esphome {
namespace vs481b {

struct PendingCommand {
    std::string command;
    uint8_t expected_lines;
    std::function<void(bool ok, const std::vector<std::string> &lines)> on_done;
    // If set, the reply is complete once a line containing this arrives, instead of after expected_lines
    const char *last_line_marker{nullptr};
};

class VS481B : public uart::UARTDevice, public PollingComponent {
    public:
        inline void register_ports(std::vector<std::string> ports) { this->ports = ports; }
        inline void register_defaults(Settings settings) 
        { 
            this->default_settings = settings;
        }
        void setup() override;
        void loop() override;
        void update() override;
        void dump_config() override;

        void set_port(int port);
        void set_output(bool state);
        void set_mode(Mode mode);
        void set_auto_port(int port);
        void set_goto(bool state);

        void set_hpd(bool state);

        inline const std::vector<std::string> &get_ports() const {
            return this->ports;
        }

        bool is_switch_available() const { return this->switch_available_; }

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
        CallbackManager<void(bool)> available_callback;

    protected:
        void enqueue_(std::string command, uint8_t expected_lines,
            std::function<void(bool ok, const std::vector<std::string> &lines)> on_done,
            const char *last_line_marker = nullptr);
        void handle_line_(std::string line);
        void finish_(bool ok);
        void apply_read_(const std::vector<std::string> &lines);
        void apply_defaults_();
        void on_read_result_(bool ok);

        std::deque<PendingCommand> queue_;
        optional<PendingCommand> current_command_;
        std::vector<std::string> response_;
        std::string line_buffer_;
        uint32_t sent_at_{0};

        // The switch counts as available after a successful read, and unavailable after
        // several failed reads in a row. Defaults are applied each time it becomes available.
        bool switch_available_{false};
        uint8_t failed_reads_{0};

        Settings current_settings;
        Settings default_settings;

        std::vector<std::string> ports;
};
}
}

#endif