#ifndef HPP_BODY
#define HPP_BODY

namespace sim
{
    class Body2d
    {
    public:
        Body2d(double mass, double x, double y, double vx, double vy);
        double mass, x, y, vx, vy;

    private:
    };
    class Body3d
    {
    public:
        Body3d(double mass, double x, double y, double z, double vx, double vy, double vz);

    private:
        double mass, x, y, z, vx, vy, vz;
    };
}

#endif