#pragma once
#include "Game/Entities/Ship.hpp"
#include "Game/Stats.hpp"
#include <stdint.h>
#include "Engine/Renderer/RGBA.hpp"

enum class PowerUpType;
class PlayerPilot;
class TextRenderable2D;
class Material;
class ShaderProgram;

class PlayerShip : public Ship
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    PlayerShip(PlayerPilot* playerPilot);
    virtual ~PlayerShip();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);

    void UpdateEquips(float deltaSeconds);

    virtual void Render() const;
    virtual void ResolveCollision(Entity* otherEntity);
    virtual void Die();
    void Respawn();
    inline virtual bool IsPlayer() { return true; }
    void DropPowerups();
    void PickUpItem(Item* pickedUpItem);
    void DropRandomPowerup();
    RGBA GetPlayerColor();
    void HideUI();
    void ShowUI();
    void InitializeUI();
    bool CanPickUp(Item* item);
    void CheckToEjectEquipment(float deltaSeconds);

    //EQUIPMENT/////////////////////////////////////////////////////////////////////
    void EjectWeapon();
    void EjectChassis();
    void EjectActive();
    void EjectPassive();

    //STAT FUNCTIONS/////////////////////////////////////////////////////////////////////
    inline virtual float GetTopSpeedStat() { return Ship::GetTopSpeedStat() + m_powerupStatModifiers.topSpeed; };
    inline virtual float GetAccelerationStat() { return Ship::GetAccelerationStat() + m_powerupStatModifiers.acceleration; };
    inline virtual float GetHandlingStat() { return Ship::GetHandlingStat() + m_powerupStatModifiers.handling; };
    inline virtual float GetBrakingStat() { return Ship::GetBrakingStat() + m_powerupStatModifiers.braking; };
    inline virtual float GetDamageStat() { return Ship::GetDamageStat() + m_powerupStatModifiers.damage; };
    inline virtual float GetShieldDisruptionStat() { return Ship::GetShieldDisruptionStat() + m_powerupStatModifiers.shieldDisruption; };
    inline virtual float GetShotHomingStat() { return Ship::GetShotHomingStat() + m_powerupStatModifiers.shotHoming; };
    inline virtual float GetRateOfFireStat() { return Ship::GetRateOfFireStat() + m_powerupStatModifiers.rateOfFire; };
    inline virtual float GetHpStat() { return Ship::GetHpStat() + m_powerupStatModifiers.hp; };
    inline virtual float GetShieldCapacityStat() { return Ship::GetShieldCapacityStat() + m_powerupStatModifiers.shieldCapacity; };
    inline virtual float GetShieldRegenStat() { return Ship::GetShieldRegenStat() + m_powerupStatModifiers.shieldRegen; };
    inline virtual float GetShotDeflectionStat() { return Ship::GetShotDeflectionStat() + m_powerupStatModifiers.shotDeflection; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Stats m_powerupStatModifiers;
    ShaderProgram* m_recolorShader = nullptr;
    Material* m_recolorMaterial = nullptr;
    Sprite* m_speedometer = nullptr;
    Sprite* m_equipUI = nullptr;
    Sprite* m_currentWeaponUI = nullptr;
    TextRenderable2D* m_healthText = nullptr;
    TextRenderable2D* m_shieldText = nullptr;
    TextRenderable2D* m_speedText = nullptr;
    TextRenderable2D* m_dpsText = nullptr;
    float m_totalDamageDone = 0.0f;
    double m_activeBeginEjectMilliseconds = -10.0f;
    double m_passiveBeginEjectMilliseconds = -10.0f;
    double m_weaponBeginEjectMilliseconds = -10.0f;
    double m_chassisBeginEjectMilliseconds = -10.0f;
};