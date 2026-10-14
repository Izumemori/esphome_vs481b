#pragma once

#include <strings.h>

namespace esphome {
namespace vs481b {

enum Mode {
    M_INVALID = -1,
    M_NEXT = 0,
    M_AUTO = 1,
    M_DEFAULT = 2
};

inline const char *mode_to_str(Mode mode) {
    switch(mode) {
        case M_NEXT:
            return "Next";
        case M_AUTO:
            return "Auto";
        case M_DEFAULT:
            return "Default";

        default:
            return "Invalid";
    }
}

inline Mode str_to_mode(const char *input) {
    if (strcasecmp(input, "next") == 0)
        return Mode::M_NEXT;

    if (strcasecmp(input, "auto") == 0)
        return Mode::M_AUTO;

    if (strcasecmp(input, "default") == 0)
        return Mode::M_DEFAULT;

    return Mode::M_INVALID;
}

} // namespace vs481b
} // namespace esphome
