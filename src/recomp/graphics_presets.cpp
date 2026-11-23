#include <rex/cvar.h>
#include <rex/logging.h>

#include <cctype>
#include <string>
#include <string_view>

namespace {

struct PresetEntry {
    const char* name;
    const char* value;
};

// Each preset is a bundle of graphics CVARs. Values are chosen to satisfy each
// CVAR's declared constraints (integer range, double range, or allowed string
// list); SetFlagByName rejects anything outside those constraints.
//
// resolution_scale and the present_* CVARs use lifecycle kRequiresRestart:
// SetFlagByName updates the stored value immediately, but the renderer only
// picks it up after a restart. The settings overlay marks those with [restart].
constexpr PresetEntry kNative[] = {
    {"resolution_scale", "1"},
    {"anisotropic_override", "-1"},
    {"present_effect", "bilinear"},
    {"present_dither", "false"},
};

constexpr PresetEntry kQuality[] = {
    {"resolution_scale", "3"},
    {"anisotropic_override", "5"},
    {"present_effect", "fsr3"},
    {"present_fsr_quality_mode", "quality"},
    {"present_cas_additional_sharpness", "0.5"},
    {"present_fsr_sharpness_reduction", "0.2"},
    {"present_dither", "true"},
};

constexpr PresetEntry kBalanced[] = {
    {"resolution_scale", "2"},
    {"anisotropic_override", "3"},
    {"present_effect", "fsr"},
    {"present_fsr_quality_mode", "balanced"},
    {"present_cas_additional_sharpness", "0.3"},
    {"present_fsr_sharpness_reduction", "0.2"},
    {"present_dither", "true"},
};

constexpr PresetEntry kPerformance[] = {
    {"resolution_scale", "4"},
    {"anisotropic_override", "1"},
    {"present_effect", "fsr"},
    {"present_fsr_quality_mode", "performance"},
    {"present_cas_additional_sharpness", "0.2"},
    {"present_fsr_sharpness_reduction", "0.2"},
    {"present_dither", "false"},
};

struct PresetDef {
    std::string_view name;
    const PresetEntry* entries;
    size_t count;
};

template <size_t N>
constexpr PresetDef MakePreset(std::string_view name, const PresetEntry (&entries)[N])
{
    return {name, entries, N};
}

constexpr PresetDef kPresets[] = {
    MakePreset("native", kNative),
    MakePreset("quality", kQuality),
    MakePreset("balanced", kBalanced),
    MakePreset("performance", kPerformance),
};

}  // namespace

// Implemented as a command rather than a CVAR with a change callback:
// SetFlagFromSource holds the registry mutex while firing change callbacks, so
// calling SetFlagByName from one would deadlock. InvokeCommand releases the
// lock before invoking the command callback, so this is safe.
REXCVAR_DEFINE_COMMAND_ARGS(
    graphics_preset,
    [](std::string_view args) {
        const auto first = args.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos) {
            REXLOG_INFO("Usage: graphics_preset <native|quality|balanced|performance>");
            return;
        }
        const auto last = args.find_last_not_of(" \t\r\n");
        std::string name(args.substr(first, last - first + 1));
        for (char& c : name)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        const PresetDef* chosen = nullptr;
        for (const auto& preset : kPresets) {
            if (preset.name == name) {
                chosen = &preset;
                break;
            }
        }
        if (!chosen) {
            REXLOG_INFO("Unknown graphics preset '{}'. Valid: native, quality, balanced, performance",
                        name);
            return;
        }

        int applied = 0;
        int failed = 0;
        for (size_t i = 0; i < chosen->count; ++i) {
            if (rex::cvar::SetFlagByName(chosen->entries[i].name, chosen->entries[i].value)) {
                ++applied;
            } else {
                ++failed;
                REXLOG_WARN("graphics_preset: could not set {} = {} (unavailable in this build?)",
                            chosen->entries[i].name, chosen->entries[i].value);
            }
        }
        REXLOG_INFO("graphics_preset '{}': {} applied, {} failed. Render settings need a restart.",
                    name, applied, failed);
    },
    "Hydra/Graphics",
    "Apply a graphics preset: native | quality | balanced | performance (render settings need a restart)");
