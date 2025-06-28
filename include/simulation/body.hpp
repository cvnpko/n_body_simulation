#ifndef HPP_BODY
#define HPP_BODY

namespace sim
{
    class Body2d
    {
    public:
        Body2d(float mass, float x, float y, float vx, float vy);
        Body2d();
        float mass, x, y, vx, vy;

    private:
    };
    class Body3d
    {
    public:
        Body3d(float mass, float x, float y, float z, float vx, float vy, float vz);
        Body3d();
        float mass, x, y, z, vx, vy, vz;

    private:
    };
}

#endif