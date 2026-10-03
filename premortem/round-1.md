# Pre-mortem round 1 — fresh-context reviewer (Opus 5.5, role-separated; A-016)

## Incident report (simulated): "6 months after v0.1.0, the plugin is pulled"
Following the plan to the letter, v0.1.0 shipped with green CI. Within weeks users reported:
1. **Reaper/Ableton crash when automating or switching the instrument during playback.** The processor called `Engine::setInstrument` (allocating, documented "message thread") from a parameter callback that some hosts deliver on the audio thread; the audio thread rendered while voices were being reallocated. Several users lost unsaved sessions.
2. **The matouqin and sihu "plucked" instead of bowing.** Step S-007 introduced placeholder plucked voices; every frozen test passed with the placeholder, so a missed family-voice hookup was invisible to CI.
3. **CPU spikes and dropouts a few seconds after releasing chords** on Intel machines — decaying waveguide tails fell into denormal range; T-080 only measured held notes.
4. The UI highlighter occasionally drew garbage positions (torn reads of `voiceInfo` across threads), and TSAN would have flagged it — but no job ran TSAN.
5. Minor: some users disliked timbres (expected; DD parameters), automation lanes broke after a parameter was renamed in a patch release.

## Lenses
| Lens | Finding |
|---|---|
| Technical correctness | (2) family voice not verified by any test; (3) denormals untested |
| Dependency / supply-chain drift | macOS runner Xcode default may change (macos-26 → newer Xcode); pins otherwise SHA-locked |
| Invalid research assumptions | R-001..R-004 (Medium, DD body data) unchanged |
| Implementer misinterpretation | (1) "message thread" contract is easy to violate inside a plugin; placeholder voices (2) |
| Integrity | No route to fabricate results found: numbers come from tests/renders; DD changes logged (DR-10) |
| Scale & performance | (3) tails; per-voice FIR up to 47 taps at 96 kHz (C-068) — covered by T-080 only at 48 kHz |
| Security | Parsers fuzzed (T-100/T-101), no network, pinned deps + OSV; no new finding |
| Operational / maintenance | (5) parameter IDs not declared stable; runner drift (DR-11) |

## Risks added (see RISK_REGISTER.md): R-005 .. R-011
