#pragma once

/**
 * Main DSP header for Chasm - includes all DSP components.
 * 
 * This header provides easy access to all DSP functionality:
 * - Schroeder Allpass Filter Chain for reverb/delay effects
 * - Stereo Enhancer for width control and frequency-dependent processing
 * - Simple filters for EQ and frequency shaping
 * - Limiter for output protection
 * - Parameter smoothing utilities
 * - Complete DSP processor
 */

// Utility classes
#include "Utils/ParameterSmoother.h"

// Filter components
#include "Filters/AllpassFilter.h"
#include "Filters/SchroederAllpassChain.h"
#include "Filters/SimpleFilter.h"

// Effect components
#include "Effects/StereoEnhancer.h"
#include "Effects/Limiter.h"

// Core DSP processor
#include "Core/ChasmProcessor.h"

namespace DSP {

/**
 * Convenience aliases for common types.
 */
using FloatProcessor = Core::ChasmProcessor<float>;
using DoubleProcessor = Core::ChasmProcessor<double>;

using FloatParameterSmoother = Utils::ParameterSmoother<float>;
using DoubleParameterSmoother = Utils::ParameterSmoother<double>;

using FloatAllpassFilter = Filters::AllpassFilter<float>;
using DoubleAllpassFilter = Filters::AllpassFilter<double>;

using FloatAllpassChain = Filters::SchroederAllpassChain<float>;
using DoubleAllpassChain = Filters::SchroederAllpassChain<double>;

using FloatSimpleFilter = Filters::SimpleFilter<float>;
using DoubleSimpleFilter = Filters::SimpleFilter<double>;

using FloatStereoEnhancer = Effects::StereoEnhancer<float>;
using DoubleStereoEnhancer = Effects::StereoEnhancer<double>;

using FloatLimiter = Effects::SmoothLimiter<float>;
using DoubleLimiter = Effects::SmoothLimiter<double>;

} // namespace DSP
