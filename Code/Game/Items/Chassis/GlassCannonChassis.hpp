#pragma once
#include "Game/Items/Chassis/Chassis.hpp"

class GlassCannonChassis : public Chassis
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    GlassCannonChassis();
    virtual ~GlassCannonChassis();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual const SpriteResource* GetSpriteResource();
    virtual const SpriteResource* GetShipSpriteResource();
    virtual void Activate(NamedProperties&) override {};
    virtual void Deactivate(NamedProperties&) override {};
};


