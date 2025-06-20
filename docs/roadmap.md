# Roadmap

June 2025

## DSP

### To Do

- Fix filters for Brightness, Low cut and High cut
- Modify character implementation to significantly reduce gain
- Implement Limiter parameters for better control
  - This will largely be a re-implementation of the existing code in MakeItLoud for end user simplicity
- Ensure mono compatible filters (use plugin doctor to test)
- fix preset manager (list doesnt update on delete even after reload)

### In Progress

- Basic licensing implementation (fix DSP when not licensed)

### Completed

- Base DSP implementation for smear effect and wet/dry mixing
