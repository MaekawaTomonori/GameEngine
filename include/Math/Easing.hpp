#pragma once

#include "Vector2.hpp"
#include "Vector3.hpp"

namespace Ease {
    // Ease In functions (start slow, accelerate)
    namespace In {
        // Quadratic easing (x^2)
        Vector2 Quad(Vector2 start, Vector2 end, float t);
        Vector3 Quad(Vector3 start, Vector3 end, float t);

        // Cubic easing (x^3)
        Vector2 Cubic(Vector2 start, Vector2 end, float t);
        Vector3 Cubic(Vector3 start, Vector3 end, float t);
    }

    // Ease Out functions (start fast, decelerate)
    namespace Out {
        // Quadratic easing
        Vector2 Quad(Vector2 start, Vector2 end, float t);
        Vector3 Quad(Vector3 start, Vector3 end, float t);

        // Cubic easing
        Vector2 Cubic(Vector2 start, Vector2 end, float t);
        Vector3 Cubic(Vector3 start, Vector3 end, float t);
    }

    // Ease InOut functions (accelerate then decelerate)
    namespace InOut {
        // Quadratic easing
        Vector2 Quad(Vector2 start, Vector2 end, float t);
        Vector3 Quad(Vector3 start, Vector3 end, float t);

        // Cubic easing
        Vector2 Cubic(Vector2 start, Vector2 end, float t);
        Vector3 Cubic(Vector3 start, Vector3 end, float t);
    }
}