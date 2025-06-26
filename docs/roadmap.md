# Roadmap

June 2025

## Chasm DSP

### To Do

- [ ] Modify character implementation to significantly reduce gain (v0.1.4)

- [ ] Ensure mono compatible filters (use plugin doctor to test)

- [ ] fix preset manager (list doesnt update on delete even after reload) (v0.1.5)
  - [x] change location of saved presets
  - [ ] check for changes after every save, delete or plugin load

- [ ] Implement stereo widening (v0.1.6)
  - [ ] Simple Haas based widening (Expand / Add wideness)
  - [ ] Boost existing sides after diffusion with M/S split (exaggerate existing wideness)

### In Progress

- [ ] Implement Limiter parameters for better control (v0.1.3)
  - [x] MakeItLoud implementation
  - [ ] Implementation into DSP Processor
  - [ ] Implementation into Plugin Processor

### Completed

- [x] Base DSP implementation for smear effect and wet/dry mixing (v0.1.0)

- [x] Basic licensing implementation (fix DSP when not licensed) (v0.1.1)

- [x] Fix filters for Brightness, Low cut and High cut (v 0.1.2)
  - [x] Make High and low cut sliders logarithmic
  - [x] Make cuts convert into flat filters near 0 and 20khz (aka turn em off)
  - [x] Implement high shelf for brightness
  - [x] fix brightness only on one side
    - [x] Use process duplicator for IIR filter since can only be used on mono


### Possible future additions

- [ ] Dropdowns to control cut slopes
- [ ] LFOs to modulate parameters in the plugin
- [ ] Visuals of filters / FX in the plugin
- [ ] A/B testing
- [ ] Multiple stereo widening modes
- [ ] 
