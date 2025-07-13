#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "simulation/body.hpp"
#include <vector>
#include <cmath>

namespace sim
{
    class QuadTree
    {
    public:
        QuadTree(float radius, float left, float right, float up, float down);
        QuadTree(float radius, float left, float right, float up, float down, int depth);
        ~QuadTree();

        void addBody(Body body);
        std::vector<float> calForce(Body body, float G, float alpha, float theta);

    private:
        int depth;
        float massCentreX, massCentreY;
        float mass, radius;
        float leftBorder, rightBorder, upBorder, downBorder;

        QuadTree *children[2][2];
        std::vector<Body> bodies;
    };
}

#endif