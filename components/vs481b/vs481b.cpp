#include "vs481b.h"

#include <cstdio>
#include <cstring>
#include <strings.h>

namespace esphome {
namespace vs481b {

static const char *const TAG = "vs481b";
static const uint32_t COMMAND_TIMEOUT_MS = 500;
static const uint32_t MAX_LINE_LENGTH = 64;
static const char *const READ_COMMAND = "read";
static const char *const READ_LAST_LINE =
    "F/W:";  // Last line of a read reply, which has an extra AutoPort line in auto mode
static const uint8_t FAILED_READS_UNTIL_UNAVAILABLE = 3;

// Queue keys, see PendingCommand::key. Mode and auto port share one: both change the mode.
static const char *const KEY_READ = "read";
static const char *const KEY_PORT = "port";
static const char *const KEY_OUTPUT = "output";
static const char *const KEY_MODE = "mode";
static const char *const KEY_GOTO = "goto";
static const char *const KEY_HPD = "hpd";

void VS481B::setup() {
  // Defaults are applied once the switch answers its first read, see on_read_result_()
  this->status_set_warning("Waiting for the switch to respond");
}

// Brings the switch to `target`: the defaults when it first answers after boot, or the state
// from before an outage when it answers again. Runs after a successful read, so only settings
// that differ from what the switch just reported are sent. A switch that rebooted gets its
// state back; after a mere connection glitch nothing needs to be sent.
void VS481B::apply_settings_(const Settings &target) {
  const Settings &now = this->current_settings;

  if (target.port.has_value() && now.port != target.port) this->set_port(*target.port);

  if (target.output_enabled.has_value() && now.output_enabled != target.output_enabled)
    this->set_output(*target.output_enabled);

  if (target.mode == Mode::M_AUTO) {
    // Auto mode is entered by selecting the auto port, so a different auto port also needs sending
    optional<int> auto_port = target.auto_port.has_value() ? target.auto_port : now.auto_port;
    if (now.mode != Mode::M_AUTO || now.auto_port != auto_port) {
      if (auto_port.has_value()) {
        this->set_auto_port(*auto_port);
      } else {
        this->set_mode(Mode::M_AUTO);  // logs that there's no auto port to switch to
      }
    }
  } else if (target.mode.has_value() && now.mode != target.mode) {
    this->set_mode(*target.mode);
  }

  if (target.goto_enabled.has_value() && now.goto_enabled != target.goto_enabled) this->set_goto(*target.goto_enabled);

  // HPD isn't in the read reply, so there's nothing to compare against: always send it
  if (target.hpd_enabled.has_value()) this->set_hpd(*target.hpd_enabled);
}

void VS481B::dump_config() {
  ESP_LOGCONFIG(TAG, "VS481B:");
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  Switch responding: %s", YESNO(this->switch_available_));

  ESP_LOGCONFIG(TAG, "  Ports:");
  for (size_t i = 0; i < this->ports.size(); i++)
    ESP_LOGCONFIG(TAG, "    %u: %s", (unsigned)(i + 1), this->ports[i].c_str());

  const Settings &defaults = this->default_settings;
  ESP_LOGCONFIG(TAG, "  Defaults:");
  if (defaults.port.has_value())
    ESP_LOGCONFIG(TAG, "    Port: %d (%s)", *defaults.port, this->ports[*defaults.port - 1].c_str());
  if (defaults.output_enabled.has_value()) ESP_LOGCONFIG(TAG, "    Output: %s", ONOFF(*defaults.output_enabled));
  if (defaults.mode.has_value()) ESP_LOGCONFIG(TAG, "    Mode: %s", mode_to_str(*defaults.mode));
  if (defaults.auto_port.has_value())
    ESP_LOGCONFIG(TAG, "    Auto port: %d (%s)", *defaults.auto_port, this->ports[*defaults.auto_port - 1].c_str());
  if (defaults.goto_enabled.has_value()) ESP_LOGCONFIG(TAG, "    Goto: %s", ONOFF(*defaults.goto_enabled));
  if (defaults.hpd_enabled.has_value()) ESP_LOGCONFIG(TAG, "    HPD: %s", ONOFF(*defaults.hpd_enabled));
  if (!defaults.port.has_value() && !defaults.output_enabled.has_value() && !defaults.mode.has_value() &&
      !defaults.auto_port.has_value() && !defaults.goto_enabled.has_value() && !defaults.hpd_enabled.has_value())
    ESP_LOGCONFIG(TAG, "    (none)");

  // Warns if the uart bus doesn't match the switch's 19200 8N1
  this->check_uart_settings(19200);
}

void VS481B::loop() {
  uint8_t c;
  while (this->available() && this->read_byte(&c)) {
    if (c == '\r' || c == '\n') {  // firmware sends "\n\r"
      if (!this->line_buffer_.empty()) {
        this->handle_line_(std::move(this->line_buffer_));
      }
      this->line_buffer_.clear();
    } else if (this->line_buffer_.size() < MAX_LINE_LENGTH) {
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
    ESP_LOGV(TAG, "Sending command: %s", this->current_command_->command.c_str());
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

  this->enqueue_(
      KEY_READ, READ_COMMAND, 0,
      [this](bool ok, const std::vector<std::string> &lines) {
        if (ok) this->apply_read_(lines);
        this->on_read_result_(ok);
      },
      READ_LAST_LINE);
}

void VS481B::on_read_result_(bool ok) {
  if (!ok) {
    if (this->failed_reads_ < FAILED_READS_UNTIL_UNAVAILABLE) this->failed_reads_++;
    if (this->switch_available_ && this->failed_reads_ >= FAILED_READS_UNTIL_UNAVAILABLE) {
      ESP_LOGW(TAG, "Switch stopped responding");
      this->settings_before_outage_ = this->current_settings;
      this->switch_available_ = false;
      this->status_set_warning("Switch not responding");
      this->available_callback.call(false);
    }
    return;
  }

  this->failed_reads_ = 0;
  if (this->switch_available_) return;

  this->switch_available_ = true;
  this->status_clear_warning();
  this->available_callback.call(true);

  if (!this->was_ever_available_) {
    ESP_LOGI(TAG, "Switch is responding, applying defaults");
    this->was_ever_available_ = true;
    this->apply_settings_(this->default_settings);
  } else {
    ESP_LOGI(TAG, "Switch is responding again, restoring the state from before the outage");
    this->apply_settings_(this->settings_before_outage_);
  }
}

// Takes over the state the switch reports. Only publishes, never sends commands back.
// Lines are matched by prefix: in auto mode the switch adds an "AutoPort:" line after "Mode:".
void VS481B::apply_read_(const std::vector<std::string> &lines) {
  for (size_t i = 1; i < lines.size(); i++) {  // lines[0] is "read Command OK"
    const char *line = lines[i].c_str();
    int port;
    char value[21];

    if (sscanf(line, " Input:port %d", &port) == 1) {
      if (port < 1 || port > (int)this->ports.size()) {
        ESP_LOGW(TAG, "Input port out of range: %s", line);
      } else if (this->current_settings.port != port) {
        this->current_settings.port = port;
        this->port_callback.call(port);
      }
    } else if (sscanf(line, " AutoPort:port %d", &port) == 1) {
      if (port < 1 || port > (int)this->ports.size()) {
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
      if (mode == Mode::M_INVALID) {
        ESP_LOGW(TAG, "Unrecognised mode in read reply: %s", line);
      } else if (this->current_settings.mode != mode) {
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
  this->enqueue_(KEY_PORT, command, 1, [this, port](bool ok, const std::vector<std::string> &) {
    if (ok) this->current_settings.port = port;

    // On failure, re-publish the old value so the select snaps back
    if (this->current_settings.port.has_value()) this->port_callback.call(*this->current_settings.port);
  });
}

void VS481B::set_mode(Mode mode) {
  if (mode == Mode::M_AUTO) {
    // Auto mode is entered by selecting the auto port
    optional<int> port = this->current_settings.auto_port.has_value() ? this->current_settings.auto_port
                                                                      : this->default_settings.auto_port;

    if (port.has_value()) return this->set_auto_port(port.value());

    ESP_LOGW(TAG, "Cannot switch to auto mode without an auto port");
    if (this->current_settings.mode.has_value()) this->mode_callback.call(*this->current_settings.mode);
    return;
  }

  const char *command = mode == Mode::M_NEXT ? "swmode next" : "swmode default";
  this->enqueue_(KEY_MODE, command, 1, [this, mode](bool ok, const std::vector<std::string> &) {
    if (ok) this->current_settings.mode = mode;

    // On failure, re-publish the old value so the select snaps back
    if (this->current_settings.mode.has_value()) this->mode_callback.call(*this->current_settings.mode);
  });
}

void VS481B::set_auto_port(int port) {
  char command[24];
  snprintf(command, sizeof(command), "swmode i%02d auto", port);
  this->enqueue_(KEY_MODE, command, 1, [this, port](bool ok, const std::vector<std::string> &) {
    if (ok) {
      this->current_settings.auto_port = port;
      this->current_settings.mode = Mode::M_AUTO;
    }

    // On failure, re-publish the old values so the selects snap back
    if (this->current_settings.auto_port.has_value()) this->auto_port_callback.call(*this->current_settings.auto_port);
    if (this->current_settings.mode.has_value()) this->mode_callback.call(*this->current_settings.mode);
  });
}

void VS481B::set_goto(bool state) {
  this->enqueue_(KEY_GOTO, state ? "swmode goto on" : "swmode goto off", 1,
                 [this, state](bool ok, const std::vector<std::string> &) {
                   if (ok) this->current_settings.goto_enabled = state;

                   // On failure, re-publish the old value so the switch snaps back
                   if (this->current_settings.goto_enabled.has_value())
                     this->goto_callback.call(*this->current_settings.goto_enabled);
                 });
}

void VS481B::set_output(bool state) {
  this->enqueue_(KEY_OUTPUT, state ? "sw on" : "sw off", 1, [this, state](bool ok, const std::vector<std::string> &) {
    if (ok) this->current_settings.output_enabled = state;

    // On failure, re-publish the old value so the switch snaps back
    if (this->current_settings.output_enabled.has_value())
      this->output_callback.call(*this->current_settings.output_enabled);
  });
}

void VS481B::set_hpd(bool state) {
  this->enqueue_(KEY_HPD, state ? "hpd on" : "hpd off", 1, [this, state](bool ok, const std::vector<std::string> &) {
    if (ok) this->current_settings.hpd_enabled = state;

    // HPD can't be read back, so this always reflects the last successful command
    if (this->current_settings.hpd_enabled.has_value()) this->hpd_callback.call(*this->current_settings.hpd_enabled);
  });
}

void VS481B::enqueue_(const char *key, std::string command, uint8_t expected_lines,
                      std::function<void(bool ok, const std::vector<std::string> &lines)> on_done,
                      const char *last_line_marker) {
  // While the switch isn't answering, each command would only wait out its timeout, and a
  // backlog would replay stale actions once it's back. Fail straight away instead, so the
  // entity snaps back. Reads still go out: they're how the switch is detected again.
  if (!this->switch_available_ && strcmp(key, KEY_READ) != 0) {
    ESP_LOGW(TAG, "Switch not responding, not sending: %s", command.c_str());
    if (on_done) on_done(false, {});
    return;
  }

  PendingCommand cmd{key, std::move(command), expected_lines, std::move(on_done), last_line_marker};

  // Newest wins: replace a queued (not yet sent) command for the same setting
  for (auto &queued : this->queue_) {
    if (strcmp(queued.key, key) == 0) {
      ESP_LOGV(TAG, "Replacing queued '%s' with '%s'", queued.command.c_str(), cmd.command.c_str());
      queued = std::move(cmd);
      return;
    }
  }

  this->queue_.push_back(std::move(cmd));
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

}  // namespace vs481b
}  // namespace esphome
