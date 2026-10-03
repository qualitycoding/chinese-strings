// FROZEN — DO NOT MODIFY. Cited anchor values the instrument data MUST reproduce.
#pragma once
#include <cmath>
namespace anchors {
inline double midiHz(int n) { return 440.0 * std::pow(2.0, (n - 69) / 12.0); }
// Erhu: inner D4 / outer A4; Samejima 2023 Table 1 (C-032): diameters 0.440/0.260 mm, T 51.68/51.50 N,
// rho 6577/8363 kg/m3, E 200 GPa; implied vibrating length 0.387 m (S-03).
struct S { int midi; double diamM, T, rho, E, Lv; };
inline constexpr S kErhu[2] = { {62, 0.440e-3, 51.68, 6577, 200e9, 0.387}, {69, 0.260e-3, 51.50, 8363, 200e9, 0.387} };
// Yehu: F4 / C5; Zheng et al. DAFx26 Table 1 (C-034); vibrating length 0.295 m (S-03).
struct Y { int midi; double r, T, rho, E, s0, s1, Lv; };
inline constexpr Y kYehu[2] = { {65, 0.55e-3, 50.61, 1250, 1.0e10, 0.9754, 0.0026, 0.295}, {72, 0.40e-3, 64.29, 1350, 9.5e9, 1.5844, 0.0032, 0.295} };
// Huqin default tunings (C-039, D-011): gaohu G4-D5; zhonghu G3-D4 (fifth below erhu; octave below gaohu).
inline constexpr int kGaohu[2] = {67, 74};
inline constexpr int kZhonghu[2] = {55, 62};
// Guzheng (C-012): 21 strings, D-major pentatonic D2 (38) .. D6 (86).
inline constexpr int kGuzheng[21] = {38,40,42,45,47, 50,52,54,57,59, 62,64,66,69,71, 74,76,78,81,83, 86};
// Decay oracle (C-063, S-06): yehu string 1 partial T60 (s), n = 1..6.
inline constexpr double kYehuS1T60[6] = {5.4380, 3.2056, 1.9034, 1.2133, 0.8276, 0.5960}; // copied from research/spikes/S-06-decay-oracle/OUTPUT.json
}
