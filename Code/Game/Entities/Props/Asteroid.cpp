#include "Game/Entities/Props/Asteroid.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"
#include "Engine/Renderer/2D/ParticleSystem.hpp"
#include "Game/Items/PowerUp.hpp"

const float Asteroid::MAX_ANGULAR_VELOCITY = 15.0f;
const float Asteroid::MIN_ASTEROID_SCALE = 0.5f;
const float Asteroid::MAX_ASTEROID_SCALE = 1.5f;

//-----------------------------------------------------------------------------------
Asteroid::Asteroid(const Vector2& initialPosition)
    : Entity()
    , m_angularVelocity(MathUtils::GetRandomFloatFromZeroTo(MAX_ANGULAR_VELOCITY) - (MAX_ANGULAR_VELOCITY * 2.0f))
{
    m_sprite = new Sprite("Asteroid", TheGame::GEOMETRY_LAYER);
    m_sprite->m_transform.SetParent(&m_transform);
    m_transform.SetScale(Vector2(MathUtils::GetRandomFloat(MIN_ASTEROID_SCALE, MAX_ASTEROID_SCALE)));
    CalculateCollisionRadius();
    SetPosition(initialPosition);
    m_transform.SetRotationDegrees(MathUtils::GetRandomFloatFromZeroTo(360.0f));
    m_collisionSpriteResource = ResourceDatabase::instance->GetSpriteResource("ParticleBrown");

    float redOffset = MathUtils::GetRandomFloat(0.9f, 1.0f);
    float greenOffset = MathUtils::GetRandomFloat(0.9f, 1.0f);
    float blueOffset = MathUtils::GetRandomFloat(0.9f, 1.0f);
    m_sprite->m_tintColor = RGBA(redOffset, greenOffset, blueOffset, 1.0f);

    m_baseStats.hp = 3.0f * m_transform.GetWorldScale().x;
    Heal();
    m_isImmobile = (m_transform.GetWorldScale().x >= MIN_ASTEROID_SCALE);
}

//-----------------------------------------------------------------------------------
Asteroid::~Asteroid()
{

}

//-----------------------------------------------------------------------------------
void Asteroid::Die()
{
    static SoundID deathSound = AudioSystem::instance->CreateOrGetSound("Data/SFX/Hit/cratePop.ogg");
    Entity::Die();
    TheGame::instance->m_currentGameMode->PlaySoundAt(deathSound, GetPosition(), 1.0f);
    ParticleSystem::PlayOneShotParticleEffect("CrateDestroyed", TheGame::BACKGROUND_PARTICLES_LAYER, Transform2D(GetPosition()));

    GameMode* gamemode = m_currentGameMode;
    ASSERT_OR_DIE(gamemode, "Current gamemode was null for an asteroid.");

    if (gamemode->m_isPlaying && m_transform.GetWorldScale().x >= (MIN_ASTEROID_SCALE / 2.0f))
    {
        Vector2 newScale = m_transform.GetWorldScale() / 2.0f;
        float randomScaleOffset1 = MathUtils::GetRandomFloat(-0.1f, 0.1f);
        float randomScaleOffset2 = MathUtils::GetRandomFloat(-0.1f, 0.1f);

        Asteroid* asteroid1 = new Asteroid(m_transform.GetWorldPosition());
        Asteroid* asteroid2 = new Asteroid(m_transform.GetWorldPosition());
        asteroid1->m_transform.SetScale(newScale + Vector2(randomScaleOffset1));
        asteroid2->m_transform.SetScale(newScale + Vector2(randomScaleOffset2));
        asteroid1->ApplyImpulse(MathUtils::GetRandomDirectionVector() * 1250.0f);
        asteroid2->ApplyImpulse(MathUtils::GetRandomDirectionVector() * 1250.0f);
        asteroid1->CalculateCollisionRadius();
        asteroid2->CalculateCollisionRadius();

        if (asteroid1->m_transform.GetWorldScale().x <= MIN_ASTEROID_SCALE)
        {
            asteroid1->m_isImmobile = false;
        }   
        if (asteroid2->m_transform.GetWorldScale().x <= MIN_ASTEROID_SCALE)
        {
            asteroid2->m_isImmobile = false;
        }

        if (gamemode->m_dropItemsOnDeath && MathUtils::CoinFlip())
        {
            gamemode->SpawnPickup(new PowerUp(), m_transform.GetWorldPosition());
        }

        gamemode->SpawnEntityInGameWorld(asteroid1);
        gamemode->SpawnEntityInGameWorld(asteroid2);
    }
}

//-----------------------------------------------------------------------------------
void Asteroid::Update(float deltaSeconds)
{
    Entity::Update(deltaSeconds);
    Vector2 pos = m_transform.GetWorldPosition();
    pos += m_velocity * deltaSeconds;
    m_velocity *= 0.9f;
    SetPosition(pos);

    float newRotationDegrees = m_transform.GetWorldRotationDegrees() + (m_angularVelocity * deltaSeconds);
    m_transform.SetRotationDegrees(newRotationDegrees);
}

//-----------------------------------------------------------------------------------
void Asteroid::RecalculateHP()
{
    m_baseStats.hp = 3.0f * m_transform.GetWorldScale().x;
    Heal();
}
