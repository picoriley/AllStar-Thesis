#include "Game/Entities/Projectile.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Input/Logging.hpp"
#include <algorithm>

//-----------------------------------------------------------------------------------
Projectile::Projectile(Entity* owner, float power, float disruption, float penetration)
    : Entity()
    , m_speed(10.0f)
    , m_power(power)
    , m_disruption(disruption)
    , m_penetration(penetration)
    , m_lifeSpan(2.0f)
{
    m_owner = owner;
    m_collidesWithBullets = false;
    m_staysWithinBounds = false;
    m_sprite = new Sprite("Laser", TheGame::PLAYER_BULLET_LAYER);
    m_sprite->m_scale = Vector2(1.0f, 1.0f);
    m_sprite->m_tintColor = owner->m_sprite->m_tintColor;
    CalculateCollisionRadius();

    SetPosition(owner->GetPosition());
    m_sprite->m_rotationDegrees = m_owner->m_sprite->m_rotationDegrees;

    Vector2 direction = Vector2::DegreesToDirection(-m_sprite->m_rotationDegrees, Vector2::ZERO_DEGREES_UP);

    float ownerForwardSpeed = Vector2::Dot(direction, m_owner->m_velocity);
    ownerForwardSpeed = std::max<float>(0.0f, ownerForwardSpeed);
    float adjustedSpeed = m_speed + ownerForwardSpeed;

    Vector2 muzzleVelocity = direction * adjustedSpeed;
    m_velocity = muzzleVelocity;
}

//-----------------------------------------------------------------------------------
Projectile::~Projectile()
{
}

//-----------------------------------------------------------------------------------
void Projectile::Update(float deltaSeconds)
{
    Entity::Update(deltaSeconds);
    if (m_age < m_lifeSpan)
    {
        Vector2 position = GetPosition();
        Vector2 velocity = m_velocity + (m_accelerationViaImpulse * deltaSeconds);
        position += velocity * deltaSeconds;
        SetPosition(position);
        m_accelerationViaImpulse = 0.0f; //Only applied for a frame.
    }
    else
    {
        m_isDead = true;
    }
}

//-----------------------------------------------------------------------------------
void Projectile::ResolveCollision(Entity* otherEntity)
{
    Entity::ResolveCollision(otherEntity);
    if (otherEntity != m_owner && otherEntity->m_collidesWithBullets && !otherEntity->m_isDead)
    {
        otherEntity->TakeDamage(m_power, m_disruption, m_penetration);
        this->m_isDead = true;
    }
}

//-----------------------------------------------------------------------------------
void Projectile::ApplyImpulse(const Vector2& appliedAcceleration)
{
    const float mass = 1.0f;
    m_accelerationViaImpulse = mass * appliedAcceleration;
}

