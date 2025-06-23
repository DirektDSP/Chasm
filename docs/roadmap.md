# Roadmap

June 2025

## DSP

### To Do

- [ ] Modify character implementation to significantly reduce gain
- [ ] Implement Limiter parameters for better control
  - [ ] This will largely be a re-implementation of the existing code in MakeItLoud for end user simplicity

- [ ] Ensure mono compatible filters (use plugin doctor to test)

- [ ] fix preset manager (list doesnt update on delete even after reload)
  - [x] change location of saved presets
  - [ ] check for changes after every save, delete or plugin load

- [ ] Implement multiple stereo widening modes (% offset, max offset of 'n' ms)
  - [ ] Simple Haas based widening (Phasing Issues)
  - [ ] Different diffusion settings / offset on L/R (Super wide)
  - [ ] Boost existing sides after diffusion with M/S split (saturation or compression on sides)

### In Progress

- [ ] Fix filters for Brightness, Low cut and High cut
  - [ ] Make High and low cut sliders logarithmic
  - [ ] Make cuts convert into flat filters near 0 and 20khz (aka turn em off)
  - [ ] Slope dropdown?
  - [ ] (uber crazy) visual of eq

### Completed

- [x] Basic licensing implementation (fix DSP when not licensed)
- [x] Base DSP implementation for smear effect and wet/dry mixing
