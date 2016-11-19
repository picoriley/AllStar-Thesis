#pragma once
#include "Game/GameModes/GameMode.hpp"

class BaseMinigameMode : public GameMode
{
public:
    BaseMinigameMode();
    virtual ~BaseMinigameMode();

    virtual void Initialize() = 0;
    virtual void Update(float deltaSeconds) { GameMode::Update(deltaSeconds); };
};