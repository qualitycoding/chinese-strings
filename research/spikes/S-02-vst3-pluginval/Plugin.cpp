// Spike S-02: minimal monophonic KS synth as VST3 (JUCE 9.0.3), validated with pluginval v1.0.4.
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
class S02 final : public juce::AudioProcessor {
public:
  S02() : AudioProcessor (BusesProperties().withOutput ("Out", juce::AudioChannelSet::stereo(), true)) {
    addParameter (gain = new juce::AudioParameterFloat ({"gain", 1}, "Gain", 0.0f, 1.0f, 0.5f)); }
  const juce::String getName() const override { return "S02 Synth"; }
  void prepareToPlay (double sr, int) override { fs = sr; line.assign ((size_t) (sr / 20.0) + 2, 0.0f); }
  void releaseResources() override {}
  bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
  void processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer& m) override {
    juce::ScopedNoDenormals nd; b.clear();
    for (const auto meta : m) { auto msg = meta.getMessage(); if (msg.isNoteOn()) pluck (msg.getNoteNumber()); }
    if (N <= 1) return;
    for (int i = 0; i < b.getNumSamples(); ++i) {
      float y = line[(size_t) idx]; float nv = 0.498f * (y + prev); prev = y; line[(size_t) idx] = nv; idx = (idx + 1) % N;
      for (int c = 0; c < b.getNumChannels(); ++c) b.setSample (c, i, y * gain->get()); } }
  bool acceptsMidi() const override { return true; } bool producesMidi() const override { return false; }
  double getTailLengthSeconds() const override { return 2.0; }
  juce::AudioProcessorEditor* createEditor() override { return new juce::GenericAudioProcessorEditor (*this); }
  bool hasEditor() const override { return true; }
  int getNumPrograms() override { return 1; } int getCurrentProgram() override { return 0; }
  void setCurrentProgram (int) override {} const juce::String getProgramName (int) override { return {}; }
  void changeProgramName (int, const juce::String&) override {}
  void getStateInformation (juce::MemoryBlock& d) override { juce::MemoryOutputStream (d, true).writeFloat (gain->get()); }
  void setStateInformation (const void* d, int s) override { if (s >= 4) *gain = juce::MemoryInputStream (d, (size_t) s, false).readFloat(); }
private:
  void pluck (int note) { double f = juce::MidiMessage::getMidiNoteInHertz (note);
    N = juce::jlimit (2, (int) line.size(), (int) (fs / f)); idx = 0;
    for (int i = 0; i < N; ++i) line[(size_t) i] = rng.nextFloat() * 2.0f - 1.0f; }
  juce::AudioParameterFloat* gain = nullptr; double fs = 44100.0; std::vector<float> line; int N = 0, idx = 0; float prev = 0; juce::Random rng { 7 };
};
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new S02(); }
