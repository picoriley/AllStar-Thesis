#pragma once
#include "Game/Entities/Ship.hpp"
#include "Game/Stats.hpp"
#include <stdint.h>
#include "Engine/Renderer/RGBA.hpp"
#include "../Items/PowerUp.hpp"
#include "../Items/Actives/WarpActive.hpp"

enum class PowerUpType;
class PlayerPilot;
class TextRenderable2D;
class BarGraphRenderable2D;
class Material;
class ShaderProgram;

class PlayerShip : public Ship
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    PlayerShip(PlayerPilot* playerPilot);
    virtual ~PlayerShip();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds) override;
    void UpdatePlayerUI(float deltaSeconds);
    void UpdateEquips(float deltaSeconds);
    virtual void Render() const;
    virtual void ResolveCollision(Entity* otherEntity) override;
    virtual float TakeDamage(float damage, float disruption = 1.0f) override;
    virtual void Drain(float drainValue) override;
    virtual void Heal(float healValue = 99999999.0f) override;
    virtual void SetShieldHealth(float newShieldValue = 99999999.0f) override;
    virtual void Die() override;
    void Respawn();
    inline virtual bool IsPlayer() { return true; }
    void DropPowerupsAndEquipment();
    void PickUpItem(Item* pickedUpItem);
    void DropRandomPowerup();
    RGBA GetPlayerColor();
    void HideUI();
    void ShowUI();
    void InitializeStatGraph();
    void ShowStatGraph();
    void SlowShowStatGraph();
    void HideStatGraph();
    void InitializeUI();
    bool CanPickUp(Item* item);
    void CheckToEjectEquipment(float deltaSeconds);
    void SetPaletteOffset(int paletteIndex);
    void DebugUpdate(float deltaSeconds);
    void LockAbilities() { m_abilitiesLocked = true; };
    void UnlockAbilities() { m_abilitiesLocked = false; };

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
    static const Vector2 DEFAULT_SCALE;
    static const char* RESPAWN_TEXT;
    static const char* DEAD_TEXT;
    static constexpr double EJECT_TIME_SECONDS = 0.5f;
    static constexpr double EJECT_TIME_MILLISECONDS = EJECT_TIME_SECONDS * 1000.0f;
    static constexpr double FULL_MESSAGE_TIME_SECONDS = 1.0f;
    static constexpr double FULL_MESSAGE_TIME_MILLISECONDS = FULL_MESSAGE_TIME_SECONDS * 1000.0f;

    WarpActive m_warpFreebieActive;
    Stats m_powerupStatModifiers;
    ShaderProgram* m_paletteSwapShader = nullptr;
    ShaderProgram* m_cooldownShader = nullptr;
    Material* m_paletteSwapMaterial = nullptr;
    Material* m_playerTintedUIMaterial = nullptr;
    Material* m_cooldownMaterial = nullptr;
    Material* m_shieldDownEffect = nullptr;
    Sprite* m_equipUI = nullptr;
    Sprite* m_playerData = nullptr;
    Sprite* m_currentWeaponUI = nullptr;
    Sprite* m_currentActiveUI = nullptr;
    Sprite* m_currentPassiveUI = nullptr;
    Sprite* m_currentChassisUI = nullptr;
    Sprite* m_statValuesBG = nullptr;
    Sprite* m_statSprites[(unsigned int)PowerUpType::NUM_POWERUP_TYPES];
    TextRenderable2D* m_respawnText = nullptr;
    TextRenderable2D* m_healthText = nullptr;
    TextRenderable2D* m_shieldText = nullptr;
    TextRenderable2D* m_tpText = nullptr;
    TextRenderable2D* m_scoreText = nullptr;
    TextRenderable2D* m_statValues[(unsigned int)PowerUpType::NUM_POWERUP_TYPES];
    BarGraphRenderable2D* m_healthBar = nullptr;
    BarGraphRenderable2D* m_teleportBar = nullptr;
    BarGraphRenderable2D* m_shieldBar = nullptr;
    BarGraphRenderable2D* m_statBarGraphs[(unsigned int)PowerUpType::NUM_POWERUP_TYPES];
    double m_activeBeginEjectMilliseconds = -10.0f;
    double m_passiveBeginEjectMilliseconds = -10.0f;
    double m_weaponBeginEjectMilliseconds = -10.0f;
    double m_chassisBeginEjectMilliseconds = -10.0f;
    double m_timeSinceFullDisplayedMilliseconds = -10.0f;
    float m_totalDamageDone = 0.0f;
    float m_tpChargeLastFrame;
    int m_rank = 0;
    int m_points = 0;
    bool m_abilitiesLocked = false;
};