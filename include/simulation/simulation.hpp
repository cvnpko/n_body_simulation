#ifndef STATES_HPP
#define STATES_HPP
namespace sim
{
    enum class States
    {
        MENU,
        ThreeBody2DInit,
        ThreeBody2DSim,
        NBodyBigInit,
        NBodyBigSim,
        NBodySmallInit,
        NBodySmallSim,
        TwoFixedBodyInit,
        TwoFixedBodySim,
        ThreeBody3DInit,
        ThreeBody3DSim
    };
}
#endif