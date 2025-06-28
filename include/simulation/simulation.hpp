#ifndef STATES_HPP
#define STATES_HPP
namespace sim
{
    enum class Option
    {
        MENU,
        ThreeBody2D,
        NBodyBig,
        NBodySmall,
        TwoFixedBody,
        ThreeBody3D
    };
    enum class States
    {
        MENU,
        Init,
        Sim
    };
}
#endif