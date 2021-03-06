#pragma once
#include "Game/Items/Actives/ActiveEffect.hpp"

class Ship;

class WarpActive : public ActiveEffect
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    WarpActive();
    virtual ~WarpActive();
    virtual void Update(float deltaSeconds);
    virtual void Activate(NamedProperties& parameters);
    virtual void Deactivate(NamedProperties& parameters);

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual const SpriteResource* GetSpriteResource();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Ship* m_transportee = nullptr;

    static const double SECONDS_UNTIL_WARP;
    static const double MILISECONDS_UNTIL_WARP;
    static constexpr float WARP_DAMAGE_PER_FRAME = 100.0f;
};
