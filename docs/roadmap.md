# Roadmap

June 2025

## Chasm DSP

### To Do

- [ ] Implement stereo widening (v0.1.6)
  - [ ] Simple Haas based widening (Expand / Add wideness)
  - [ ] Boost existing sides after diffusion with M/S split (exaggerate existing wideness)

### In Progress

- [ ] fix preset manager (list doesnt update on delete even after reload) (v0.1.5)
  - [x] change location of saved presets
  - [ ] check for changes after every save, delete or plugin load

### Completed

- [x] Base DSP implementation for smear effect and wet/dry mixing (v0.1.0)

- [x] Basic licensing implementation (fix DSP when not licensed) (v0.1.1)

- [x] Fix filters for Brightness, Low cut and High cut (v 0.1.2)
  - [x] Make High and low cut sliders logarithmic
  - [x] Make cuts convert into flat filters near 0 and 20khz (aka turn em off)
  - [x] Implement high shelf for brightness
  - [x] fix brightness only on one side
    - [x] Use process duplicator for IIR filter since can only be used on mono

- [x] Implement Limiter parameters for better control (v0.1.3)
  - [x] MakeItLoud implementation
  - [x] Implementation into DSP Processor
  - [x] Implementation into Plugin Processor

- [x] Modify character implementation to significantly reduce gain (v0.1.4)
  - this is problematic since character is the feedback parameter for the allpass chains.
  - We cant have too low a value since it does nothing but too high and its clipping time.

### Possible future additions

- [ ] Dropdowns to control cut slopes
- [ ] LFOs to modulate parameters in the plugin
- [ ] Visuals of filters / FX in the plugin
- [ ] A/B testing
- [ ] Multiple stereo widening modes
- [ ] Skins
- [ ] Online DB of plugins
- [ ] 

## Chasm UI

A main note for the ui, the general theme is some overlap between a chrome, cybersigilism inspired aesthetic and a natural aesthetic.

It would make sense to split the ui in half and have one half react using chrome based effects and the other half with nature inspired ones.

UI Component layout

- Main delay know *must* be a main element -> nature, Centre of UI, large
- high/low cut -> chrome, centre bottom
- Brightness and Character controls -> chrome, left
- In and Output gain controls -> nature, right
- MakeItLoud components, unique to plugin -> mix of themes, flat modern, bottom.

### To Do (UI)

- [ ] Main UI
  - [ ] Misc Knobs
    - [ ] Preset Dropdown
    - [ ] Save Button
    - [ ] Delete Button

### In Progress (UI)

- MakeItLoud UI Elements
  - [ ] Dropdown menus
  - [ ] Knobs
  - [ ] Toggle

### Done (UI)

- [x] Main Delay Knob
- [x] Sliders
  - [x] Main Sliders
  - [x] Dual High/Low Cut Slider

### Exports to Manage

- Filmstrips for UI elements
  - Knobs
  - Sliders
  - Buttons
  - Info UI Elements (TBD)

- Blender Files
  - Main UI Implementation
  - Info UI (TBD)
