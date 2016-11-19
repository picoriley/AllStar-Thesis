#pragma once
#include "Game/Entities/Entity.hpp"

class Sprite;
class Pilot;
class Vector2;

class Ship : public Entity
{
public:
    Ship(Pilot* pilot = nullptr);
    virtual ~Ship();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void ResolveCollision(Entity* otherEntity);
    virtual void LockMovement() { m_lockMovement = true; };
    virtual void UnlockMovement() { m_lockMovement = false; };
    virtual void ToggleMovement() { m_lockMovement = !m_lockMovement; };
    void AttemptMovement(const Vector2& attemptedPosition);
    void UpdateMotion(float deltaSeconds);
    void UpdateShooting();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Pilot* m_pilot;
    float m_timeSinceLastShot;
    bool m_lockMovement = false;
};