#include "Game/GameModes/AssemblyMode.hpp"
#include "Game/Entities/PlayerShip.hpp"
#include "Game/TheGame.hpp"
#include "Game/Pilots/PlayerPilot.hpp"
#include "Game/Entities/ItemCrate.hpp"
#include "Game/Entities/Grunt.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"

//-----------------------------------------------------------------------------------
AssemblyMode::AssemblyMode()
    : GameMode()
{
    for (unsigned int i = 0; i < TheGame::instance->m_playerPilots.size(); ++i)
    {
        PlayerShip* player = new PlayerShip(TheGame::instance->m_playerPilots[i]);
        m_players.push_back(player);
        m_entities.push_back(player);
    }
    ItemCrate* box1 = new ItemCrate(Vector2(2.0f));
    ItemCrate* box2 = new ItemCrate(Vector2(1.0f));
    Grunt* g1 = new Grunt(Vector2(-2.0f));
    Grunt* g2 = new Grunt(Vector2(-1.0f));

    m_entities.push_back(box1);
    m_entities.push_back(box2);
    m_entities.push_back(g1);
    m_entities.push_back(g2);
}

//-----------------------------------------------------------------------------------
AssemblyMode::~AssemblyMode()
{
    for (Entity* ent : m_entities)
    {
        delete ent;
    }
    m_entities.clear();
}

//-----------------------------------------------------------------------------------
void AssemblyMode::Update(float deltaSeconds)
{
    m_timeSinceLastSpawn += deltaSeconds;
    if (m_timeSinceLastSpawn > TIME_PER_SPAWN)
    {
        m_entities.push_back(new ItemCrate(GetRandomLocationInArena()));
        m_entities.push_back(new Grunt(GetRandomLocationInArena()));
        m_timeSinceLastSpawn = 0.0f;
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
    for (auto iter = m_entities.begin(); iter != m_entities.end(); ++iter)
    {
        Entity* gameObject = *iter;
        if (gameObject->m_isDead && !gameObject->IsPlayer())
        {
            delete gameObject;
            iter = m_entities.erase(iter);
        }
        if (iter == m_entities.end())
        {
            break;
        }
    }

    for (unsigned int i = 0; i < m_players.size(); ++i)
    {
        SpriteGameRenderer::instance->SetCameraPosition(m_players[i]->GetPosition(), i);
    }
}
