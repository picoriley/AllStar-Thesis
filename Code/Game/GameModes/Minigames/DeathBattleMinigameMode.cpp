#include "Game/GameModes/Minigames/DeathBattleMinigameMode.hpp"
#include "Game/TheGame.hpp"
#include "Game/Entities/PlayerShip.hpp"
#include "Game/Entities/Props/Asteroid.hpp"
#include "Game/Pilots/Pilot.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Game/Encounters/Encounter.hpp"
#include "Game/Encounters/NebulaEncounter.hpp"
#include "Game/Encounters/BossteroidEncounter.hpp"
#include "Engine/Renderer/2D/TextRenderable2D.hpp"
#include "Game/Encounters/BlackHoleEncounter.hpp"
#include "Game/Entities/Props/BlackHole.hpp"

//-----------------------------------------------------------------------------------
DeathBattleMinigameMode::DeathBattleMinigameMode()
    : BaseMinigameMode()
{
    m_gameLengthSeconds = 120.0f;
    m_respawnAllowed = false;
    if (!g_disableMusic)
    {
        m_backgroundMusic = AudioSystem::instance->CreateOrGetSound("Data/Music/Foxx - Sweet Tooth - 03 Sorbet.ogg");
    }
    m_modeTitleText = "DEATH BATTLE";
    m_modeDescriptionText = "No Respawns, Get as many kills as you can!";
    m_readyBGColor = RGBA::JOLTIK_PURPLE;
    m_readyTextColor = RGBA::JOLTIK_YELLOW;

    HideBackground();
}

//-----------------------------------------------------------------------------------
DeathBattleMinigameMode::~DeathBattleMinigameMode()
{

}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::Initialize(const std::vector<PlayerShip*>& players)
{
    SetBackground("BattleBackground", Vector2(25.0f));
    SpriteGameRenderer::instance->CreateOrGetLayer(TheGame::BACKGROUND_LAYER)->m_virtualScaleMultiplier = 10.0f;
    SpriteGameRenderer::instance->SetWorldBounds(AABB2(Vector2(-20.0f), Vector2(20.0f)));
    GameMode::Initialize(players);

    InitializePlayerData();
    SpawnGeometry();
    SpawnPlayers();
    m_isPlaying = true;
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::CleanUp()
{
    GameMode::CleanUp();
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::SpawnPlayers()
{
    for (PlayerShip* player : m_players)
    {
        m_entities.push_back(player);
        player->Respawn();
        player->m_scoreText->Enable();
    }
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::SpawnGeometry()
{
    //Add in some Asteroids (for color)
    for (int i = 0; i < 20; ++i)
    {
        m_entities.push_back(new Asteroid(GetRandomLocationInArena()));
    }

    //Spawn the all-consuming black-hole
    float radius = 0.5f;
    RemoveEntitiesInCircle(Vector2::ZERO, radius);
    BlackHoleEncounter* blackHole = new BlackHoleEncounter(Vector2::ZERO, radius);
    blackHole->Spawn();
    blackHole->m_spawnedBlackHole->m_growsOverTime = true;
    m_encounters.push_back(blackHole);

    SetVortexPositions();

}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::UpdatePlayerScoreDisplay(PlayerShip* player)
{
    int numKills = m_playerStats[player]->m_numKills;
    player->m_scoreText->m_text = Stringf("Kills: %03i", numKills);
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::InitializePlayerData()
{
    for (PlayerShip* player : m_players)
    {
        DeathBattleStats* stats = new DeathBattleStats(player);
        stats->m_timeAlive = 0.0f;
        m_playerStats[player] = stats;
    }
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::RecordPlayerDeath(PlayerShip* ship)
{
    GameMode::RecordPlayerDeath(ship);
    ((DeathBattleStats*)m_playerStats[ship])->m_timeAlive = GetTimerSecondsElapsed();
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::Update(float deltaSeconds)
{
    BaseMinigameMode::Update(deltaSeconds);
    deltaSeconds = m_scaledDeltaSeconds;

    if (!m_isPlaying)
    {
        return;
    }
    for (Entity* ent : m_entities)
    {
        ent->Update(deltaSeconds);
        for (Entity* other : m_entities)
        {
            if ((ent != other) && (ent->IsCollidingWith(other)))
            {
                ent->ResolveCollision(other);
            }
        }
    }
    for (Entity* ent : m_newEntities)
    {
        m_entities.push_back(ent);
    }
    m_newEntities.clear();
    for (auto iter = m_entities.begin(); iter != m_entities.end();)
    {
        Entity* gameObject = *iter;
        if (gameObject->m_isDead && !gameObject->IsPlayer())
        {
            delete gameObject;
            iter = m_entities.erase(iter);
            continue;
        }
        if (iter == m_entities.end())
        {
            break;
        }
        ++iter;
    }
    
    EndGameIfTooFewPlayers();
    for (PlayerShip* player : m_players)
    {
        UpdatePlayerScoreDisplay(player);
    }

    UpdatePlayerCameras();
    SetVortexPositions();
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::RankPlayers()
{
    float* scores = new float[TheGame::instance->m_numberOfPlayers];
    for (unsigned int i = 0; i < m_players.size(); ++i)
    {
        scores[i] = INT_MIN;
        PlayerShip* ship = m_players[i];
        DeathBattleStats* stats = (DeathBattleStats*)m_playerStats[ship];
        float timeAliveBonus = stats->m_timeAlive == 0.0f ? 10000.0f : stats->m_timeAlive;
        scores[i] = (float)stats->m_numKills + timeAliveBonus;
        ship->m_rank = 999;
    }
    for (unsigned int i = 0; i < m_players.size(); ++i)
    {
        int numBetterPlayers = 0;
        int myScore = scores[i];
        for (unsigned int j = 0; j < m_players.size(); ++j)
        {
            int otherScore = scores[j];
            if (myScore < otherScore)
            {
                ++numBetterPlayers;
            }
        }
        m_players[i]->m_rank = numBetterPlayers + 1; //1st place has 0 people better
    }
    delete scores;
}

//-----------------------------------------------------------------------------------
Encounter* DeathBattleMinigameMode::GetRandomMinorEncounter(const Vector2& center, float radius)
{
    int random = MathUtils::GetRandomIntFromZeroTo(4);
    switch (random)
    {
    case 0:
    case 1:
        return new NebulaEncounter(center, radius);
    case 2:
    case 3:
        return new BossteroidEncounter(center, radius);
    default:
        ERROR_AND_DIE("Random medium encounter roll out of range");
    }
}

//-----------------------------------------------------------------------------------
void DeathBattleMinigameMode::SetUpPlayerSpawnPoints()
{
    //Random start positions
}