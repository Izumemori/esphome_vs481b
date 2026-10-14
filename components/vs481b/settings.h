#pragma once

#include <string>

#include "esphome/core/optional.h"

#include "mode.h"

namespace esphome {
namespace vs481b {
struct Settings {
  optional<int> port;
  optional<bool> output_enabled;
  optional<Mode> mode;
  optional<bool> goto_enabled;
  optional<int> auto_port;
  std::string fw_version;
  optional<bool> hpd_enabled;
};
}  // namespace vs481b
}  // namespace esphome
