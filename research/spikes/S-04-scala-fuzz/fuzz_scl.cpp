// Spike S-04: libFuzzer + ASan/UBSan over tuning-library parseSCLData/parseKBMData
// through the size-limited loader proposed in D-010 (max 64 KiB, max 1024 notes).
#include "Tunings.h"
#include <cstdint>
#include <string>
static bool loadSclLimited(const std::string& s) {
  if (s.size() > 65536) return false;
  try { auto sc = Tunings::parseSCLData(s); if (sc.count > 1024) return false;
        Tunings::Tuning t(sc); (void) t.frequencyForMidiNote(60); return true; }
  catch (const Tunings::TuningError&) { return false; }
}
static bool loadKbmLimited(const std::string& s) {
  if (s.size() > 65536) return false;
  try { auto k = Tunings::parseKBMData(s); (void) k; return true; }
  catch (const Tunings::TuningError&) { return false; }
}
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* d, size_t n) {
  std::string s(reinterpret_cast<const char*>(d), n);
  loadSclLimited(s); loadKbmLimited(s); return 0;
}
