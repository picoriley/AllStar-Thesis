#pragma once
#include "Game/Items/Item.hpp"
#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------------
enum class PowerUpType
{
    TOP_SPEED = 0,
    ACCELERATION,
    HANDLING,
    BRAKING,
    DAMAGE,
    SHIELD_DISRUPTION,
    SHOT_HOMING,
    RATE_OF_FIRE,
    HP,
    SHIELD_CAPACITY,
    SHIELD_REGEN,
    SHOT_DEFLECTION,
    NUM_POWERUP_TYPES,
    RANDOM
};

//-----------------------------------------------------------------------------------
class PowerUp : public Item
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    PowerUp(PowerUpType type = PowerUpType::RANDOM);
    PowerUp(Stats statChanges);
    virtual ~PowerUp();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual const SpriteResource* GetSpriteResource() override;
    virtual void Activate(NamedProperties&) override {};
    virtual void Deactivate(NamedProperties&) override {};
    const char* GetPowerUpSpriteResourceName();
    void ApplyPickupEffect(PlayerShip* player);
    void SetStatChangeFromType(PowerUpType type);
    SoundID GetPickupSFXID();

    static const char* GetPowerUpSpriteResourceName(PowerUpType type);
    static RGBA GetPowerUpColor(PowerUpType type);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    PowerUpType m_powerUpType;
    Stats m_statChanges;
};