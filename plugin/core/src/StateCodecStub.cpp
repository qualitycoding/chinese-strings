// STUB — replaced during implementation.
#include "cs/StateCodec.h"
#include "cs/Errors.h"
namespace cs {
std::vector<std::uint8_t> encodeState(const PluginState&) { throw NotImplemented("encodeState"); }
std::optional<PluginState> decodeState(const void*, int) { throw NotImplemented("decodeState"); }
}
