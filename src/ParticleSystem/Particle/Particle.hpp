#ifndef Particle_HPP_
#define Particle_HPP_
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

class Particle {
    struct ForGpu {
        Vector3 position;
        Vector3 scale;
        float lifetime;
        Vector3 velocity;
        float timer;
        Vector4 color;
    };

public:
    
private:

}; // class Particle

#endif // Particle_HPP_
