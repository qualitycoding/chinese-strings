// Public interface (D-016, D-020). Implementation uses JUCE XML (copyXmlToBinary / getXmlFromBinary).
#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>
namespace cs {
inline constexpr int kStateVersion = 1;
struct PluginState {
    int version = kStateVersion;
    std::string instrumentKey = "erhu";
    std::map<std::string, float> params;   // keys = cs::paramKey(...)
    std::string sclText, kbmText;          // dropped (set empty) if either exceeds 65536 bytes or fails cs::loadScala
    bool operator==(const PluginState&) const = default;
};
std::vector<std::uint8_t> encodeState(const PluginState& s);
// Never throws. nullopt for: size<=8, wrong magic, unparsable XML, root != <cs-state>, version > kStateVersion,
// unknown instrument key. Params: unknown keys ignored, values clamped to cs::paramRange.
std::optional<PluginState> decodeState(const void* data, int sizeBytes);
}
