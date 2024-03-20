#include "vs481b.h"
#include <cmath>

namespace esphome{
namespace vs481b {

static const char *TAG = "vs481b";

uint32_t VS481B::get_time_per_char() {
    return (uint32_t)ceill(1000000.0/this->parent_->get_baud_rate()) * (this->parent_->get_data_bits() + this->parent_->get_stop_bits());
}

bool VS481B::send_command(std::string command, bool should_flush) {
    write_str(command.c_str());

    if (command.find("\r\n") == std::string::npos)
        write_str("\r\n");

    std::string result_string =  this->read_line();

    ESP_LOGD(TAG, "Got response: %s", result_string.c_str());

    uint8_t buffer;
    if (should_flush)
         while (available()) read_byte(&buffer);

    return result_string.find("OK") != std::string::npos;
}

std::string VS481B::read_line() {
    std::string response;

    uint8_t c;

    if (!read_byte(&c)) return std::string();
    while(c >= 0 && c != '\r') // Bug in firmware, sends \n\r instead of \r\n
    {
        if (!available())
            delay_microseconds_safe(this->get_time_per_char());

        response += c;
        read_byte(&c);
    }

    response.erase(response.begin(), std::find_if(response.begin(), response.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));

    response.erase(std::find_if(response.rbegin(), response.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), response.end());

    return response;
}

void VS481B::update() {
    if (!this->has_been_setup){
        if (this->default_settings.port.has_value())
            set_port(this->default_settings.port.value());

        if (this->default_settings.output_enabled.has_value())
            set_output(this->default_settings.output_enabled.value());

        if (this->default_settings.mode.has_value())
            set_mode(this->default_settings.mode.value());

        if (this->default_settings.goto_enabled.has_value())
            set_goto(this->default_settings.goto_enabled.value());

        set_hpd(this->default_settings.hpd_enabled);

        this->has_been_setup = true;
    }

    std::string command = "read";
    if (!send_command(command, false)) return;

    std::string port_read = this->read_line();
    std::string output_read = this->read_line();
    std::string mode_read = this->read_line();
    std::string goto_read = this->read_line();
    std::string fw_read = this->read_line();

    int port;
    sscanf(port_read.c_str(), "Input:port %2d", &port);
    this->set_port(port);

    char output_res[4];
    sscanf(output_read.c_str(), "Output:%3s", &output_res);
    this->set_output(output_res[1] == 'N');

    char mode[10];
    sscanf(mode_read.c_str(), "Mode:%10s", &mode);
    this->set_mode(str_to_mode(mode));

    char goto_res[4];
    sscanf(goto_read.c_str(), "Goto:%3s", &goto_res);
    this->set_goto(goto_res[1] == 'N');

    char fw_res[20];
    sscanf(fw_read.c_str(), "F/W:%20s", &fw_res);
    this->set_fw(fw_res);
}

bool VS481B::set_port(int port) {
    if (port < 1) port = 1;
    if (port > this->ports.size()) port = this->ports.size();

    if (this->current_settings.port == port) return true;

    char command[7]; // sw i<port>
    sprintf(command, "sw i%02d\0", port);

    if (!send_command(command)) return false; // Didn't work! Don't update

    this->port_callback.call(port);

    this->current_settings.port = port;

    return true;
}

bool VS481B::set_mode(Mode mode) {
    if (this->current_settings.mode == mode) return true;

    std::string command;
    if (mode == Mode::M_AUTO){
        if (!this->current_settings.auto_port.has_value()
            && !this->default_settings.auto_port.has_value()) return false;

        int port = this->current_settings.auto_port.value_or(this->default_settings.auto_port.value());

        return this->set_auto_port(port);
    }
    else if (mode == Mode::M_NEXT)
        command = "swmode next";
    else command = "swmode default";

    if (!send_command(command)) return false; // Didn't work! Don't update

    this->mode_callback.call(mode);

    this->current_settings.mode = mode;

    return true;
}

bool VS481B::set_auto_port(int port) {
    if (this->current_settings.auto_port == port) return true;

    char command[16];
    sprintf(command, "swmode i%02d auto", this->current_settings.auto_port.value());

    if (!send_command(command)) return false; // Didn't work! Don't update


    if (this->current_settings.mode != Mode::M_AUTO){
        this->mode_callback.call(Mode::M_AUTO);
        this->current_settings.mode = Mode::M_AUTO;
    }
    this->auto_port_callback.call(port);
    this->current_settings.auto_port = port;

    return true;
}

bool VS481B::set_goto(bool state) {
    if (this->current_settings.goto_enabled == state) return true;

    std::string command;
    if (state)
        command = "swmode goto on";
    else command = "swmode goto off";

    if (!send_command(command)) return false; // Didn't work! Don't update

    this->goto_callback.call(state);

    this->current_settings.goto_enabled = state;

    return true;
}

bool VS481B::set_output(bool state) {
    if (this->current_settings.output_enabled == state) return true;

    std::string command;
    if (state)
        command = "sw on";
    else command = "sw off";

    if (!send_command(command)) return false; // Didn't work! Don't update

    this->output_callback.call(state);

    this->current_settings.output_enabled = state;

    return true;
}

bool VS481B::set_fw(std::string fw) {
    if (this->current_settings.fw_version == fw) return true;

    this->fw_callback.call(fw);

    this->current_settings.fw_version = fw;

    return true;
}

bool VS481B::set_hpd(bool state) {
    std::string command;
    if (state)
        command = "hpd on";
    else command = "hpd off";

    if (!send_command(command)) return false; // Didn't work! Don't update

    this->hpd_callback.call(state);

    this->current_settings.hpd_enabled = state;

    return true;
}

}
}