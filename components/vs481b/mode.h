#ifndef MODE_H
#define MODE_H

namespace esphome {
namespace vs481b {

enum Mode {
    M_NEXT = 0,
    M_AUTO = 1,
    M_DEFAULT = 2
};

inline std::string mode_to_str(Mode mode) {
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

inline Mode str_to_mode(std::string input) {
    std::string lower;
    for (auto& c : input)
        lower += std::tolower(c);
    
    if (lower == "next")
        return Mode::M_NEXT;

    if (lower == "auto")
        return Mode::M_AUTO;

    return Mode::M_DEFAULT;
}

}
}

#endif