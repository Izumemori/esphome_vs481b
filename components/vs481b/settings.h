#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#include "mode.h"

namespace esphome
{
namespace vs481b
{
    struct Settings {
        optional<int> port;
        optional<bool> output_enabled;
        optional<Mode> mode;
        optional<bool> goto_enabled;
        optional<int> auto_port;
        std::string fw_version;
        bool hpd_enabled;
    } typedef Settings;
} // namespace vs481b
} // namespace esphome

#endif