#include <algorithm> // std::clamp
#include <cmath>     // std::expm1

namespace math {
    // Compute the smoothing factor alpha for exponential smoothing over time.
    // alpha in [0,1]; lambda in 1/seconds; dt in seconds
    inline float expSmoothingFactor(float lambda, float dt) {
        if (dt <= 0.f || lambda <= 0.f)
            return 0.f;                            // no advance if no time or invalid rate
        const float a = -std::expm1(-lambda * dt); // numerically stable 1 - exp(-x)
        return std::clamp(a, 0.f, 1.f);
    }
} // namespace math