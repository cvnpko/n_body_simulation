#include "simulation/body.hpp"
namespace sim
{
    Body2d::Body2d(double mass, double x, double y, double vx, double vy)
        : mass(mass), x(x), y(y), vx(vx), vy(vy) {}
    Body3d::Body3d(double mass, double x, double y, double z, double vx, double vy, double vz)
        : mass(mass), x(x), y(y), z(z), vx(vx), vy(vy), vz(vz) {}
}