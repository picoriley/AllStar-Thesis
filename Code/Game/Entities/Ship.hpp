#pragma once
#include "Game/Entities/Entity.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Game/Items/Weapons/LaserGun.hpp"

class Sprite;
class Pilot;
class Vector2;
class ParticleSystem;

class Ship : public Entity
{
public:
    Ship(Pilot* pilot = nullptr);
    virtual ~Ship();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void ResolveCollision(Entity* otherEntity);
    virtual void LockMovement() { m_lockMovement = true; m_velocity = Vector2::ZERO; };
    virtual void UnlockMovement() { m_lockMovement = false; };
    virtual void ToggleMovement() { m_lockMovement = !m_lockMovement; };
    virtual float TakeDamage(float damage, float disruption = 1.0f) override;
    virtual void Heal(float healValue = 99999999.0f) override;
    virtual void Die();
    virtual Vector2 GetMuzzlePosition();
    virtual const SpriteResource* GetCollisionSpriteResource() override;
    virtual bool FlushParticleTrailIfExists();
    void UpdateMotion(float deltaSeconds);
    void UpdateShooting();
    void RegenerateShield(float deltaSeconds);
    void ApplyShotDeflection();
    void FlickerShield(float deltaSeconds);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static constexpr float ANGULAR_VELOCITY = 300.0f;
    static constexpr float DEADZONE_BEFORE_ROTATION = 0.3f;
    static constexpr float DEADZONE_BEFORE_ROTATION_SQUARED = DEADZONE_BEFORE_ROTATION * DEADZONE_BEFORE_ROTATION;

    LaserGun m_defaultWeapon;
    Pilot* m_pilot;
    ParticleSystem* m_shipTrail = nullptr;
    ParticleSystem* m_smokeDamage = nullptr;
    const SpriteResource* m_shieldCollisionSpriteResource = nullptr;
    float m_muzzleOffsetMagnitude = 0.25f;
    float m_secondsSinceLastFiredWeapon;
    bool m_lockMovement = false;
    RGBA m_factionColor = RGBA::WHITE;
    RGBA m_factionAltColor = RGBA::GBWHITE;
};