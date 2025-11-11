#ifndef ParticleSystem_HPP_
#define ParticleSystem_HPP_
#include <memory>

#include "Emitter/Emitter.hpp"
#include "Math/Vector3.hpp"


class ParticleSystem {
    struct Group {
        Vector3 position;
        std::unique_ptr<Emitter> emitter;
    };



public:

private:

}; // class ParticleSystem

#endif // ParticleSystem_HPP_
