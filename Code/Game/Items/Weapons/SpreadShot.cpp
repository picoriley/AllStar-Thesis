#include "Game/Items/Weapons/SpreadShot.hpp"
#include "Game/Entities/Ship.hpp"
#include "Game/Pilots/Pilot.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Renderer/2D/ParticleSystem.hpp"
#include "Game/Entities/Projectiles/Projectile.hpp"
#include "Game/Entities/Projectiles/Laser.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
SpreadShot::SpreadShot()
    : Weapon()
{
    m_name = "Spread Shot";

    m_numProjectilesPerShot = 4;
    m_spreadDegrees = 75.0f;
    m_statBonuses.damage = -5.0f;
    m_statBonuses.rateOfFire = -3.0f;
}

//-----------------------------------------------------------------------------------
SpreadShot::~SpreadShot()
{

}

//-----------------------------------------------------------------------------------
const SpriteResource* SpreadShot::GetSpriteResource()
{
    return ResourceDatabase::instance->GetSpriteResource("SpreadShot");
}

//-----------------------------------------------------------------------------------
bool SpreadShot::AttemptFire(Ship* shooter)
{
    static const SoundID bulletSound = AudioSystem::instance->CreateOrGetSound("Data/SFX/Bullets/SFX_Weapon_Fire_Missle_01.ogg");
    static float RUMBLE_PERCENTAGE = 0.1f;
    static float SECONDS_TO_RUMBLE = 0.075f;
    bool successfullyFired = false;
    float secondsPerWeaponFire = 1.0f / shooter->CalculateRateOfFireValue();

    if (shooter->m_secondsSinceLastFiredWeapon > secondsPerWeaponFire)
    {
        GameMode* currentGameMode = TheGame::instance->m_currentGameMode;

        float halfSpreadDegrees = m_spreadDegrees / 2.0f;
        for (unsigned int i = 0; i < m_numProjectilesPerShot; ++i)
        {
            float degreesOffset = MathUtils::GetRandomFloat(-halfSpreadDegrees, halfSpreadDegrees);
            Projectile* bullet = (Projectile*)new Laser(shooter, degreesOffset, shooter->CalculateDamageValue(), shooter->CalculateShieldDisruptionValue(), shooter->CalculateShotHomingValue());
            bullet->m_lifeSpan = LIFE_SPAN_PER_PROJECTILE;
            currentGameMode->SpawnBullet(bullet);
            if (shooter->IsPlayer())
            {
                bullet->m_reportDPSToPlayer = true;
            }
        }
        if (shooter->m_pilot)
        {
            shooter->m_pilot->LightRumble(RUMBLE_PERCENTAGE, SECONDS_TO_RUMBLE);
            shooter->m_pilot->RecoilScreenshake(0.0f, -0.05f * Vector2::DegreesToDirection(-shooter->m_transform.GetWorldRotationDegrees(), Vector2::ZERO_DEGREES_UP));
        }
        shooter->m_secondsSinceLastFiredWeapon = 0.0f;
        successfullyFired = true;

        Vector2 shotPosition = shooter->GetMuzzlePosition();
        currentGameMode->PlaySoundAt(bulletSound, shotPosition, TheGame::BULLET_VOLUME, MathUtils::GetRandomFloat(0.9f, 1.1f));
        ParticleSystem::PlayOneShotParticleEffect("MuzzleFlash", TheGame::BULLET_LAYER_BLOOM, Transform2D(shotPosition));
    }
    return successfullyFired;
}

