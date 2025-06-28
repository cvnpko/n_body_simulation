#ifndef HPP_BODY
#define HPP_BODY

#include <vector>

namespace sim
{
    class Body
    {
    public:
        Body(int dimension);
        int dimension;
        float mass;
        std::vector<float> coord, veloc;
    };
}

#endif