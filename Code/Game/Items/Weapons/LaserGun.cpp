#include "Game/Items/Weapons/LaserGun.hpp"
#include "Game/Entities/Ship.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Renderer/2D/ParticleSystem.hpp"
#include "Game/Entities/Projectiles/Projectile.hpp"
#include "Game/Entities/Projectiles/Laser.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
LaserGun::LaserGun()
    : Weapon()
{
    m_name = "Laser Blaster";
}

//-----------------------------------------------------------------------------------
LaserGun::~LaserGun()
{

}

//-----------------------------------------------------------------------------------
const SpriteResource* LaserGun::GetSpriteResource()
{
    return ResourceDatabase::instance->GetSpriteResource("DefaultWeapon");
}

//-----------------------------------------------------------------------------------
bool LaserGun::AttemptFire(Ship* shooter)
{
    static SoundID bulletSound = AudioSystem::instance->CreateOrGetSound("Data/SFX/Bullets/SFX_Weapon_Fire_Single_02.wav");
    bool successfullyFired = false;
    float secondsPerWeaponFire = 1.0f / shooter->CalculateRateOfFireValue();

    if (shooter->m_secondsSinceLastFiredWeapon > secondsPerWeaponFire)
    {
        GameMode* currentGameMode = TheGame::instance->m_currentGameMode;

        Projectile* bullet = (Projectile*)new Laser(shooter, 0.0f, shooter->CalculateDamageValue(), shooter->CalculateShieldDisruptionValue(), shooter->CalculateShotHomingValue());            
        if (shooter->IsPlayer())
        {
            bullet->m_reportDPSToPlayer = true;
        }
        currentGameMode->SpawnBullet(bullet);
        shooter->m_secondsSinceLastFiredWeapon = 0.0f;
        successfullyFired = true;

        Vector2 shotPosition = shooter->GetMuzzlePosition();
        currentGameMode->PlaySoundAt(bulletSound, shotPosition, 0.5f);
        ParticleSystem::PlayOneShotParticleEffect("MuzzleFlash", TheGame::BULLET_LAYER_BLOOM, Transform2D(shotPosition));
    }
    return successfullyFired;
}

