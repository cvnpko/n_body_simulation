#include "simulation/body.hpp"
namespace sim
{
    Body2d::Body2d(float mass, float x, float y, float vx, float vy)
        : mass(mass), x(x), y(y), vx(vx), vy(vy) {}
    Body2d::Body2d()
        : mass(0.1f), x(0.0f), y(0.0f), vx(0.0f), vy(0.0f) {}
    Body3d::Body3d(float mass, float x, float y, float z, float vx, float vy, float vz)
        : mass(mass), x(x), y(y), z(z), vx(vx), vy(vy), vz(vz) {}
    Body3d::Body3d()
        : mass(0.1f), x(0.0f), y(0.0f), z(0.0f), vx(0.0f), vy(0.0f), vz(0.0f) {}

}