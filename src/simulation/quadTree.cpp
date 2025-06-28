#include "simulation/quadTree.hpp"

namespace sim
{
    QuadTree::~QuadTree()
    {
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                delete children[i][j];
                children[i][j] = nullptr;
            }
        }
    }
    QuadTree::QuadTree(float left, float right, float up, float down)
        : depth(10), mass(0), leftBorder(left), rightBorder(right), upBorder(up), downBorder(down)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                children[i][j] = nullptr;
            }
        }
    }
    QuadTree::QuadTree(float left, float right, float up, float down, int depth)
        : depth(depth), mass(0), leftBorder(left), rightBorder(right), upBorder(up), downBorder(down)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                children[i][j] = nullptr;
            }
        }
    }
    void QuadTree::addBody(Body body)
    {
        if (mass == 0)
        {
            mass = body.mass;
            massCentreX = body.coord[0];
            massCentreY = body.coord[1];
        }
        else
        {
            massCentreX = (body.coord[0] * body.mass + mass * massCentreX) / (mass + body.mass);
            massCentreY = (body.coord[1] * body.mass + mass * massCentreY) / (mass + body.mass);
            mass += body.mass;
        }
        if (depth != 1)
        {
            float sHor = (leftBorder + rightBorder) / 2.0f;
            float sVer = (upBorder + downBorder) / 2.0f;
            if (body.coord[0] <= sHor)
            {
                if (body.coord[1] <= sVer)
                {
                    if (children[0][0] == nullptr)
                    {
                        children[0][0] = new QuadTree(leftBorder, sHor, sVer, downBorder, depth - 1);
                    }
                    children[0][0]->addBody(body);
                }
                else
                {
                    if (children[1][0] == nullptr)
                    {
                        children[1][0] = new QuadTree(leftBorder, sHor, upBorder, sVer, depth - 1);
                    }
                    children[1][0]->addBody(body);
                }
            }
            else
            {
                if (body.coord[1] <= sVer)
                {
                    if (children[0][1] == nullptr)
                    {
                        children[0][1] = new QuadTree(sHor, rightBorder, sVer, downBorder, depth - 1);
                    }
                    children[0][1]->addBody(body);
                }
                else
                {
                    if (children[1][1] == nullptr)
                    {
                        children[1][1] = new QuadTree(sHor, rightBorder, upBorder, sVer, depth - 1);
                    }
                    children[1][1]->addBody(body);
                }
            }
        }
        else
        {
            bodies.push_back(body);
        }
    }
    std::vector<float> QuadTree::calForce(Body body, float G, float alpha, float theta)
    {
        if (mass == 0)
        {
            return {0, 0};
        }
        if (depth == 1)
        {
            std::vector<float> ret(2, 0);
            for (int i = 0; i < bodies.size(); i++)
            {
                float dx = bodies[i].coord[0] - body.coord[0];
                float dy = bodies[i].coord[1] - body.coord[1];
                float distSqr = dx * dx + dy * dy + alpha * alpha;
                float invDist = 1.0 / sqrt(distSqr);
                float invDist3 = invDist * invDist * invDist;

                ret[0] += G * bodies[i].mass * dx * invDist3;
                ret[1] += G * bodies[i].mass * dy * invDist3;
            }
            return ret;
        }
        if (body.coord[0] >= leftBorder && body.coord[0] <= rightBorder &&
            body.coord[1] >= downBorder && body.coord[1] <= upBorder)
        {
            std::vector<float> ret(2, 0);
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    if (children[i][j] != nullptr)
                    {
                        std::vector<float> tmp = children[i][j]->calForce(body, G, alpha, theta);
                        ret[0] += tmp[0];
                        ret[1] += tmp[1];
                    }
                }
            }
            return ret;
        }
        float s = sqrtf((body.coord[0] - massCentreX) * (body.coord[0] - massCentreX) +
                        (body.coord[1] - massCentreY) * (body.coord[1] - massCentreY));
        if ((upBorder - downBorder) / s <= theta)
        {
            std::vector<float> ret(2, 0);
            float dx = massCentreX - body.coord[0];
            float dy = massCentreY - body.coord[1];
            float distSqr = dx * dx + dy * dy + alpha * alpha;
            float invDist = 1.0 / sqrt(distSqr);
            float invDist3 = invDist * invDist * invDist;

            ret[0] += G * mass * dx * invDist3 / 1000.0;
            ret[1] += G * mass * dy * invDist3 / 1000.0;
            return ret;
        }
        else
        {
            std::vector<float> ret(2, 0);
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    if (children[i][j] != nullptr)
                    {
                        std::vector<float> tmp = children[i][j]->calForce(body, G, alpha, theta);
                        ret[0] += tmp[0];
                        ret[1] += tmp[1];
                    }
                }
            }
            return ret;
        }
    }
}