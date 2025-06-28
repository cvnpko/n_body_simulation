#include "simulation/body.hpp"
namespace sim
{
    Body::Body(int dimension) : dimension(dimension), mass(0.1f)
    {
        coord = std::vector<float>(dimension, 0);
        veloc = std::vector<float>(dimension, 0);
    }

}