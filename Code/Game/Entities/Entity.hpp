#pragma once
#include "Engine/Math/Transform2D.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Game/Stats.hpp"
#include <vector>

class Sprite;
class SpriteResource;
class Weapon;
class ActiveEffect;
class PassiveEffect;
class Chassis;
class Item;
class GameMode;

class Entity
{
public:
    Entity();
    virtual ~Entity();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void ResolveCollision(Entity* otherEntity);
    virtual void ApplyImpulse(const Vector2& appliedAcceleration);
    virtual float TakeDamage(float damage, float disruption = 1.0f);
    virtual void Die() { m_isDead = true; SetShieldHealth(0.0f); };
    virtual void CalculateCollisionRadius();
    virtual void SetPosition(const Vector2& newPosition);
    virtual void SetRotation(const float newDegreesRotation);
    virtual void Heal(float healValue = 99999999.0f);
    virtual void DropInventory();
    virtual void SetShieldHealth(float newShieldValue = 99999999.0f);
    virtual bool FlushParticleTrailIfExists() { return false; };
    void InitializeInventory(unsigned int inventorySize);
    void DeleteInventory();

    //QUERIES/////////////////////////////////////////////////////////////////////
    inline virtual bool IsPlayer() { return false; };
    inline virtual bool IsProp() { return false; };
    inline virtual bool IsProjectile() { return false; };
    inline virtual bool IsPickup() { return false; };
    inline virtual bool HasShield() { return m_currentShieldHealth > 0.0f; };
    inline virtual bool IsDead() const { return m_isDead; };
    inline virtual bool IsAlive() const { return !m_isDead; };
    inline virtual bool ShowsDamageNumbers() { return true; };
    inline virtual Vector2 GetPosition() { return m_transform.GetWorldPosition(); };
    inline virtual float GetRotation() { return m_transform.GetWorldRotationDegrees(); };
    inline virtual Vector2 GetMuzzlePosition() { return GetPosition(); };
    virtual bool IsCollidingWith(Entity* otherEntity);
    inline virtual bool CanTakeContactDamage() { return m_timeSinceLastHit > SECONDS_BETWEEN_CONTACT_HITS; };
    inline virtual const SpriteResource* GetCollisionSpriteResource() { return m_collisionSpriteResource; };

    //STAT FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual float GetTopSpeedStat();
    virtual float GetAccelerationStat();
    virtual float GetHandlingStat();
    virtual float GetBrakingStat();
    virtual float GetDamageStat();
    virtual float GetShieldDisruptionStat();
    virtual float GetShotHomingStat();
    virtual float GetRateOfFireStat();
    virtual float GetHpStat();
    virtual float GetShieldCapacityStat();
    virtual float GetShieldRegenStat();
    virtual float GetShotDeflectionStat();

    //STAT VALUE FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual float CalculateTopSpeedValue();
    virtual float CalculateAccelerationValue();
    virtual float CalculateHandlingValue();
    virtual float CalculateBrakingValue();
    virtual float CalculateDamageValue();
    virtual float CalculateShieldDisruptionValue();
    virtual float CalculateShotHomingValue();
    virtual float CalculateRateOfFireValue();
    virtual float CalculateHpValue();
    virtual float CalculateShieldCapacityValue();
    virtual float CalculateShieldRegenValue();
    virtual float CalculateShotDeflectionValue();

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static Vector2 SHIELD_SCALE_FUDGE_VALUE;
    static constexpr float SECONDS_BETWEEN_CONTACT_HITS = 1.0f / 16.0f;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Stats m_baseStats;
    Weapon* m_weapon;
    ActiveEffect* m_activeEffect;
    PassiveEffect* m_passiveEffect;
    Chassis* m_chassis;
    GameMode* m_currentGameMode = nullptr;

    const SpriteResource* m_collisionSpriteResource = nullptr;
    Sprite* m_sprite;
    Sprite* m_shieldSprite;
    Entity* m_owner;
    Transform2D m_transform;
    Vector2 m_velocity;
    Vector2 m_sumOfImpulses = Vector2::ZERO;
    std::vector<Item*> m_inventory;
    double m_timeLastWarped = 0.0;
    float m_currentHp;
    float m_collisionRadius;
    float m_age;
    float m_timeSinceLastHit = 0.0f;
    float m_frictionValue;
    float m_currentShieldHealth;
    float m_mass = 1.0f;
    float m_collisionDamageAmount = 0.0f;
    bool m_isDead = false;
    bool m_collidesWithBullets = true;
    bool m_noCollide = false;
    bool m_isInvincible = false;
    bool m_staysWithinBounds = true;
    bool m_isImmobile = false;
};