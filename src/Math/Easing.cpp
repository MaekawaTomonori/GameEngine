#include "Math/Easing.hpp"

namespace Ease {
    namespace In {
        // ===== Quadratic Easing (x^2) =====
        Vector2 Quad(Vector2 start, Vector2 end, float t) {
            return start + (end - start) * t * t;
        }

        Vector3 Quad(Vector3 start, Vector3 end, float t) {
            return start + (end - start) * t * t;
        }

        // ===== Cubic Easing (x^3) =====
        Vector2 Cubic(Vector2 start, Vector2 end, float t) {
            return start + (end - start) * t * t * t;
        }

        Vector3 Cubic(Vector3 start, Vector3 end, float t) {
            return start + (end - start) * t * t * t;
        }
    }

    namespace Out {
        // ===== Quadratic Easing =====
        Vector2 Quad(Vector2 start, Vector2 end, float t) {
            return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t));
        }

        Vector3 Quad(Vector3 start, Vector3 end, float t) {
            return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t));
        }

        // ===== Cubic Easing =====
        Vector2 Cubic(Vector2 start, Vector2 end, float t) {
            return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t));
        }

        Vector3 Cubic(Vector3 start, Vector3 end, float t) {
            return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t));
        }
    }

    namespace InOut {
        // ===== Quadratic Easing =====
        Vector2 Quad(Vector2 start, Vector2 end, float t) {
            float easedT;
            if (t < 0.5f)
                easedT = 2.0f * t * t; // EaseIn for first half
            else
                easedT = 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f; // EaseOut for second half

            return start + (end - start) * easedT;
        }

        Vector3 Quad(Vector3 start, Vector3 end, float t) {
            float easedT;
            if (t < 0.5f)
                easedT = 2.0f * t * t;
            else
                easedT = 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;

            return start + (end - start) * easedT;
        }

        // ===== Cubic Easing =====
        Vector2 Cubic(Vector2 start, Vector2 end, float t) {
            float easedT;
            if (t < 0.5f)
                easedT = 4.0f * t * t * t; // EaseIn for first half
            else
                easedT = 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f; // EaseOut for second half

            return start + (end - start) * easedT;
        }

        Vector3 Cubic(Vector3 start, Vector3 end, float t) {
            float easedT;
            if (t < 0.5f)
                easedT = 4.0f * t * t * t;
            else
                easedT = 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;

            return start + (end - start) * easedT;
        }
    }
}