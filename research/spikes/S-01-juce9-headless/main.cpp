// Spike S-01: verify JUCE 9.0.3 builds headless on Linux (no X11) with juce_dsp and
// juce_audio_processors_headless, and that a Karplus-Strong string hits pitch within 3 cents.
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors_headless/juce_audio_processors_headless.h>
#include <cmath>
#include <cstdio>
#include <vector>
int main()
{
    const double fs = 48000.0, f0 = 293.66; // erhu inner string D4
    // fractional-delay KS with first-order allpass tuning
    const double loopDelay = fs / f0 - 0.5;           // averaging filter adds 0.5 samples
    const int N = (int) std::floor (loopDelay);
    const double frac = loopDelay - N;
    const double a = (1.0 - frac) / (1.0 + frac);
    std::vector<double> buf ((size_t) N, 0.0);
    juce::Random rng (42);
    for (auto& x : buf) x = rng.nextDouble() * 2.0 - 1.0;
    const int len = (int) fs * 2;
    std::vector<float> out ((size_t) len);
    double prev = 0, apx1 = 0, apy1 = 0; int idx = 0;
    for (int n = 0; n < len; ++n)
    {
        double y = buf[(size_t) idx];
        double avg = 0.4995 * (y + prev); prev = y;
        double ap = a * avg + apx1 - a * apy1; apx1 = avg; apy1 = ap;
        buf[(size_t) idx] = ap; idx = (idx + 1) % N; out[(size_t) n] = (float) y;
    }
    // pitch by FFT peak + parabolic interpolation over 1 s (skip attack)
    const int order = 16, size = 1 << order;
    juce::dsp::FFT fft (order);
    std::vector<float> data ((size_t) size * 2, 0.0f);
    juce::dsp::WindowingFunction<float> win ((size_t) size, juce::dsp::WindowingFunction<float>::hann);
    for (int i = 0; i < size && 4800 + i < len; ++i) data[(size_t) i] = out[(size_t) (4800 + i)];
    win.multiplyWithWindowingTable (data.data(), (size_t) size);
    fft.performFrequencyOnlyForwardTransform (data.data());
    int lo = (int) (200.0 * size / fs), hi = (int) (400.0 * size / fs), k = lo;
    for (int i = lo; i < hi; ++i) if (data[(size_t) i] > data[(size_t) k]) k = i;
    double l = std::log (data[(size_t) k-1]), c = std::log (data[(size_t) k]), r = std::log (data[(size_t) k+1]);
    double p = 0.5 * (l - r) / (l - 2*c + r);
    double fEst = (k + p) * fs / size;
    double cents = 1200.0 * std::log2 (fEst / f0);
    std::printf ("JUCE %s  f0=%.3f est=%.3f err=%.3f cents\n", juce::SystemStats::getJUCEVersion().toRawUTF8(), f0, fEst, cents);
    return std::abs (cents) < 3.0 ? 0 : 1;
}
