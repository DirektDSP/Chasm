#pragma once

/**
 * Main DSP header for Chasm - includes all DSP components.
 * 
 * This header provides easy access to all DSP functionality:
 * - Schroeder Allpass Filter Chain for reverb/delay effects
 * - Stereo Enhancer for width control and frequency-dependent processing
 * - Parameter smoothing utilities
 */

// Utility classes
#include "Utils/ParameterSmoother.h"

// Filter components
#include "Filters/AllpassFilter.h"
#include "Filters/SchroederAllpassChain.h"

// Effect components
#include "Effects/StereoEnhancer.h"

// Core DSP processor
#include "Core/ChasmDSPProcessor.h"

namespace DSP {

/**
 * Convenience aliases for common types.
 */
using FloatProcessor = Core::ChasmDSPProcessor<float>;
using DoubleProcessor = Core::ChasmDSPProcessor<double>;

using FloatParameterSmoother = Utils::ParameterSmoother<float>;
using DoubleParameterSmoother = Utils::ParameterSmoother<double>;

using FloatAllpassFilter = Filters::AllpassFilter<float>;
using DoubleAllpassFilter = Filters::AllpassFilter<double>;

using FloatAllpassChain = Filters::SchroederAllpassChain<float>;
using DoubleAllpassChain = Filters::SchroederAllpassChain<double>;

using FloatStereoEnhancer = Effects::StereoEnhancer<float>;
using DoubleStereoEnhancer = Effects::StereoEnhancer<double>;

} // namespace DSP
