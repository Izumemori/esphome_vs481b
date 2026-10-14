#include "vs481b.h"

#include <cstdio>
#include <strings.h>

namespace esphome{
namespace vs481b {

static const char *TAG = "vs481b";
static const uint32_t COMMAND_TIMEOUT_MS = 500;
static const uint32_t MAX_LINE_LENGTH = 64;
static const char *READ_COMMAND = "read";
static const char *READ_LAST_LINE = "F/W:"; // Last line of a read reply, which has an extra AutoPort line in auto mode
static const uint8_t FAILED_READS_UNTIL_UNAVAILABLE = 3;

void VS481B::setup() {
    // Defaults are applied once the switch answers its first read, see on_read_result_()
    this->status_set_warning("Waiting for the switch to respond");
}

// Called when the switch becomes available: after boot, or after it stopped answering
// (power cycled or disconnected). A rebooted switch is back on its own defaults, so ours
// are sent again.
void VS481B::apply_defaults_() {
    if (this->default_settings.port.has_value())
        this->set_port(this->default_settings.port.value());

    if (this->default_settings.output_enabled.has_value())
        this->set_output(this->default_settings.output_enabled.value());

    if (this->default_settings.mode.has_value())
        this->set_mode(this->default_settings.mode.value());

    if (this->default_settings.goto_enabled.has_value())
        this->set_goto(this->default_settings.goto_enabled.value());

    if (this->default_settings.hpd_enabled.has_value())
        this->set_hpd(this->default_settings.hpd_enabled.value());
}

void VS481B::loop() {
    uint8_t c;
    while (this->available() && this->read_byte(&c)) {
        if (c == '\r' || c == '\n') { // firmware sends "\n\r"
            if (!this->line_buffer_.empty()) {
                this->handle_line_(std::move(this->line_buffer_));
            }
            this->line_buffer_.clear();
        } 
        else if (this->line_buffer_.size() < MAX_LINE_LENGTH) {
            this->line_buffer_ += static_cast<char>(c);
        }
        
    }

    if (this->current_command_.has_value() && millis() - this->sent_at_ > COMMAND_TIMEOUT_MS) {
        ESP_LOGW(TAG, "Timeout waiting for response to command: %s", this->current_command_->command.c_str());
        this->line_buffer_.clear();
        this->finish_(false);
    }

    if (!this->current_command_.has_value() && !this->queue_.empty()) {
        this->current_command_ = std::move(this->queue_.front());
        this->queue_.pop_front();
        ESP_LOGD(TAG, "Sending command: %s", this->current_command_->command.c_str());
        this->response_.clear();
        this->write_str(this->current_command_->command.c_str());
        this->write_str("\r\n");
        this->sent_at_ = millis();
    }
}

void VS481B::update() {
    // Don't stack up reads while the switch is slow or not answering
    if (this->current_command_.has_value() && this->current_command_->command == READ_COMMAND) return;
    for (const auto &cmd : this->queue_)
        if (cmd.command == READ_COMMAND) return;

    this->enqueue_(READ_COMMAND, 0, [this](bool ok, const std::vector<std::string> &lines) {
        if (ok)
            this->apply_read_(lines);
        this->on_read_result_(ok);
    }, READ_LAST_LINE);
}

void VS481B::on_read_result_(bool ok) {
    if (!ok) {
        if (this->failed_reads_ < FAILED_READS_UNTIL_UNAVAILABLE)
            this->failed_reads_++;
        if (this->switch_available_ && this->failed_reads_ >= FAILED_READS_UNTIL_UNAVAILABLE) {
            ESP_LOGW(TAG, "Switch stopped responding");
            this->switch_available_ = false;
            this->status_set_warning("Switch not responding");
            this->available_callback.call(false);
        }
        return;
    }

    this->failed_reads_ = 0;
    if (this->switch_available_)
        return;

    ESP_LOGI(TAG, "Switch is responding, applying defaults");
    this->switch_available_ = true;
    this->status_clear_warning();
    this->available_callback.call(true);
    this->apply_defaults_();
}

// Takes over the state the switch reports. Only publishes, never sends commands back.
// Lines are matched by prefix: in auto mode the switch adds an "AutoPort:" line after "Mode:".
void VS481B::apply_read_(const std::vector<std::string> &lines) {
    for (size_t i = 1; i < lines.size(); i++) { // lines[0] is "read Command OK"
        const char *line = lines[i].c_str();
        int port;
        char value[21];

        if (sscanf(line, " Input:port %d", &port) == 1) {
            if (port < 1 || port > (int) this->ports.size()) {
                ESP_LOGW(TAG, "Input port out of range: %s", line);
            } else if (this->current_settings.port != port) {
                this->current_settings.port = port;
                this->port_callback.call(port);
            }
        } else if (sscanf(line, " AutoPort:port %d", &port) == 1) {
            if (port < 1 || port > (int) this->ports.size()) {
                ESP_LOGW(TAG, "Auto port out of range: %s", line);
            } else if (this->current_settings.auto_port != port) {
                this->current_settings.auto_port = port;
                this->auto_port_callback.call(port);
            }
        } else if (sscanf(line, " Output:%3s", value) == 1) {
            bool state = strcasecmp(value, "on") == 0;
            if (this->current_settings.output_enabled != state) {
                this->current_settings.output_enabled = state;
                this->output_callback.call(state);
            }
        } else if (sscanf(line, " Mode:%10s", value) == 1) {
            Mode mode = str_to_mode(value);
            if (this->current_settings.mode != mode) {
                this->current_settings.mode = mode;
                this->mode_callback.call(mode);
            }
        } else if (sscanf(line, " Goto:%3s", value) == 1) {
            bool state = strcasecmp(value, "on") == 0;
            if (this->current_settings.goto_enabled != state) {
                this->current_settings.goto_enabled = state;
                this->goto_callback.call(state);
            }
        } else if (sscanf(line, " F/W:%20s", value) == 1) {
            if (this->current_settings.fw_version != value) {
                this->current_settings.fw_version = value;
                this->fw_callback.call(this->current_settings.fw_version);
            }
        } else {
            ESP_LOGW(TAG, "Unrecognised line in read reply: %s", line);
        }
    }
}

void VS481B::set_port(int port) {
    char command[16];
    snprintf(command, sizeof(command), "sw i%02d", port);
    this->enqueue_(command, 1, [this, port](bool ok, const std::vector<std::string> &) {
        if (ok)
            this->current_settings.port = port;
        
        // On failure, re-publish the old value so the select snaps back
        if (this->current_settings.port.has_value())
            this->port_callback.call(*this->current_settings.port);
    });
}

void VS481B::set_mode(Mode mode) {
    if (mode == Mode::M_AUTO) {
        // Auto mode is entered by selecting the auto port
        optional<int> port = this->current_settings.auto_port.has_value()
            ? this->current_settings.auto_port
            : this->default_settings.auto_port;

        if (port.has_value())
            return this->set_auto_port(port.value());

        ESP_LOGW(TAG, "Cannot switch to auto mode without an auto port");
        if (this->current_settings.mode.has_value())
            this->mode_callback.call(*this->current_settings.mode);
        return;
    }

    const char *command = mode == Mode::M_NEXT ? "swmode next" : "swmode default";
    this->enqueue_(command, 1, [this, mode](bool ok, const std::vector<std::string> &) {
        if (ok)
            this->current_settings.mode = mode;

        // On failure, re-publish the old value so the select snaps back
        if (this->current_settings.mode.has_value())
            this->mode_callback.call(*this->current_settings.mode);
    });
}

void VS481B::set_auto_port(int port) {
    char command[24];
    snprintf(command, sizeof(command), "swmode i%02d auto", port);
    this->enqueue_(command, 1, [this, port](bool ok, const std::vector<std::string> &) {
        if (ok) {
            this->current_settings.auto_port = port;
            this->current_settings.mode = Mode::M_AUTO;
        }

        // On failure, re-publish the old values so the selects snap back
        if (this->current_settings.auto_port.has_value())
            this->auto_port_callback.call(*this->current_settings.auto_port);
        if (this->current_settings.mode.has_value())
            this->mode_callback.call(*this->current_settings.mode);
    });
}

void VS481B::set_goto(bool state) {
    this->enqueue_(state ? "swmode goto on" : "swmode goto off", 1, [this, state](bool ok, const std::vector<std::string> &) {
        if (ok)
            this->current_settings.goto_enabled = state;

        // On failure, re-publish the old value so the switch snaps back
        if (this->current_settings.goto_enabled.has_value())
            this->goto_callback.call(*this->current_settings.goto_enabled);
    });
}

void VS481B::set_output(bool state) {
    this->enqueue_(state ? "sw on" : "sw off", 1, [this, state](bool ok, const std::vector<std::string> &) {
        if (ok)
            this->current_settings.output_enabled = state;

        // On failure, re-publish the old value so the switch snaps back
        if (this->current_settings.output_enabled.has_value())
            this->output_callback.call(*this->current_settings.output_enabled);
    });
}

void VS481B::set_hpd(bool state) {
    this->enqueue_(state ? "hpd on" : "hpd off", 1, [this, state](bool ok, const std::vector<std::string> &) {
        if (ok)
            this->current_settings.hpd_enabled = state;

        // HPD can't be read back, so this always reflects the last successful command
        if (this->current_settings.hpd_enabled.has_value())
            this->hpd_callback.call(*this->current_settings.hpd_enabled);
    });
}

void VS481B::enqueue_(std::string command, uint8_t expected_lines,
    std::function<void(bool ok, const std::vector<std::string> &lines)> on_done,
    const char *last_line_marker) {
    this->queue_.push_back(PendingCommand{std::move(command), expected_lines, std::move(on_done), last_line_marker});
}

void VS481B::handle_line_(std::string line) {
    if (!this->current_command_.has_value()) {
        ESP_LOGV(TAG, "Ignoring line received with no command in progress: %s", line.c_str());
        return;
    }

    // A reply starts with the echoed command ("sw i01 Command OK"). Anything before that is a
    // status message (e.g. "Auto mode Active" after switching input) or a late reply to an
    // earlier command that timed out.
    if (this->response_.empty() && line.rfind(this->current_command_->command, 0) != 0) {
        ESP_LOGV(TAG, "Ignoring line before reply to '%s': %s", this->current_command_->command.c_str(), line.c_str());
        return;
    }

    ESP_LOGV(TAG, "Received: %s", line.c_str());
    this->response_.push_back(std::move(line));
    if (this->response_.size() == 1 && this->response_[0].find("OK") == std::string::npos) {
        return this->finish_(false);
    }
    const PendingCommand &cmd = *this->current_command_;
    bool complete = cmd.last_line_marker != nullptr
        ? this->response_.back().find(cmd.last_line_marker) != std::string::npos
        : this->response_.size() >= cmd.expected_lines;
    if (complete) {
        return this->finish_(true);
    }
}

void VS481B::finish_(bool ok) {
    auto cmd = std::move(*this->current_command_);
    this->current_command_.reset();
    if (!ok && !this->response_.empty())
        ESP_LOGW(TAG, "Command '%s' failed, first reply line: %s", cmd.command.c_str(), this->response_[0].c_str());
    if (cmd.on_done) {
        cmd.on_done(ok, std::move(this->response_));
    }
}

}
}
