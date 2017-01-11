#include "Game/TheGame.hpp"
#include "Game/StateMachine.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Input/Logging.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixStack4x4.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/TextRendering/TextBox.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Input/XMLUtils.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/Entities/PlayerShip.hpp"
#include "Game/Entities/Projectile.hpp"
#include "Game/Entities/Ship.hpp"
#include "Entities/Props/ItemCrate.hpp"
#include "Entities/Grunt.hpp"
#include "Entities/Pickup.hpp"
#include "Engine/Input/InputDevices/KeyboardInputDevice.hpp"
#include "Engine/Input/InputDevices/MouseInputDevice.hpp"
#include "Pilots/PlayerPilot.hpp"
#include "Engine/Input/InputDevices/XInputDevice.hpp"
#include "Engine/Input/InputValues.hpp"
#include "GameModes/AssemblyMode.hpp"
#include "GameModes/Minigames/BattleRoyaleMinigameMode.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Core/Events/EventSystem.hpp"

TheGame* TheGame::instance = nullptr;

const float TheGame::TIME_BEFORE_PLAYERS_CAN_ADVANCE_UI = 0.5f;

//-----------------------------------------------------------------------------------
TheGame::TheGame()
    : m_currentGameMode(nullptr)
    , SFX_UI_ADVANCE(AudioSystem::instance->CreateOrGetSound("Data/SFX/UI/UI_Select_01.wav"))
{
    ResourceDatabase::instance = new ResourceDatabase();
    RegisterSprites();
    RegisterParticleEffects();
    SetGameState(GameState::MAIN_MENU);
    InitializeMainMenuState();
    EventSystem::RegisterObjectForEvent("StartGame", this, &TheGame::PressStart);

//     Material* mat = new Material(
//         new ShaderProgram("Data\\Shaders\\fixedVertexFormat.vert", "Data\\Shaders\\Post\\falcoPixelation.frag"),
//         RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
//         );
//     mat->SetFloatUniform("gPixelationFactor", 1.0f);
//     SpriteGameRenderer::instance->AddEffectToLayer(mat, UI_LAYER);
}


//-----------------------------------------------------------------------------------
TheGame::~TheGame()
{
    SetGameState(GameState::SHUTDOWN);

    if (m_currentGameMode)
    {
        m_currentGameMode->CleanUp();
        delete m_currentGameMode;
    }
    unsigned int numModes = m_queuedMinigameModes.size();
    for (unsigned int i = 0; i < numModes; ++i)
    {
        GameMode* mode = m_queuedMinigameModes.front();
        delete mode;
        m_queuedMinigameModes.pop();
    }
    for (PlayerPilot* pilot : m_playerPilots)
    {
        delete pilot;
    }
    m_playerPilots.clear();
    for (PlayerShip* ship : m_players)
    {
        delete ship;
    }
    m_players.clear();
    delete ResourceDatabase::instance;
    ResourceDatabase::instance = nullptr;
}

//-----------------------------------------------------------------------------------
void TheGame::Update(float deltaSeconds)
{
    g_secondsInState += deltaSeconds;
    SpriteGameRenderer::instance->Update(deltaSeconds);
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::TILDE))
    {
        Console::instance->ToggleConsole();
    }
    if (Console::instance->IsActive())
    {
        return;
    }
   
    switch (GetGameState())
    {
    case MAIN_MENU:
        UpdateMainMenu(deltaSeconds);
        break;
    case PLAYER_JOIN:
        UpdatePlayerJoin(deltaSeconds);
        break;
    case ASSEMBLY_GET_READY:
        UpdateAssemblyGetReady(deltaSeconds);
        break;
    case ASSEMBLY_PLAYING:
        UpdateAssemblyPlaying(deltaSeconds);
        break;
    case ASSEMBLY_RESULTS:
        UpdateAssemblyResults(deltaSeconds);
        break;
    case GAME_RESULTS_SCREEN:
        UpdateGameOver(deltaSeconds);
        break;
    case MINIGAME_GET_READY:
        UpdateMinigameGetReady(deltaSeconds);
        break;
    case MINIGAME_PLAYING:
        UpdateMinigamePlaying(deltaSeconds);
        break;
    case MINIGAME_RESULTS:
        UpdateMinigameResults(deltaSeconds);
        break;
    default:
        break;

    }
}

//-----------------------------------------------------------------------------------
void TheGame::Render() const
{
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_viewStack);
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_projStack);
    if (Console::instance->IsActive())
    {
        Console::instance->Render();
    }
    switch (GetGameState())
    {
    case MAIN_MENU:
        RenderMainMenu();
        break;
    case PLAYER_JOIN:
        RenderPlayerJoin();
        break;
    case ASSEMBLY_GET_READY:
        RenderAssemblyGetReady();
        break;
    case ASSEMBLY_PLAYING:
        RenderAssemblyPlaying();
        break;
    case ASSEMBLY_RESULTS:
        RenderAssemblyResults();
        break;
    case GAME_RESULTS_SCREEN:
        RenderGameOver();
        break;
    case MINIGAME_GET_READY:
        RenderMinigameGetReady();
        break;
    case MINIGAME_PLAYING:
        RenderMinigamePlaying();
        break;
    case MINIGAME_RESULTS:
        RenderMinigameResults();
        break;
    default:
        break;

    }

}

//-----------------------------------------------------------------------------------
//MAIN MENU/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeMainMenuState()
{
    m_titleText = new Sprite("TitleText", PLAYER_LAYER);
    m_titleText->m_scale = Vector2(10.0f, 10.0f);
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupMainMenuState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupMainMenuState(unsigned int)
{
    delete m_titleText;
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateMainMenu(float)
{
    bool keyboardStart = InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER) || InputSystem::instance->WasKeyJustPressed(' ');
    bool controllerStart = InputSystem::instance->WasButtonJustPressed(XboxButton::START) || InputSystem::instance->WasButtonJustPressed(XboxButton::A);
    if (keyboardStart || controllerStart)
    {
        PressStart(NamedProperties());
    }
}

//-----------------------------------------------------------------------------------
void TheGame::PressStart(NamedProperties& properties)
{
    SetGameState(PLAYER_JOIN);
    InitializePlayerJoinState();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderMainMenu() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::CERULEAN);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
void TheGame::EnqueueMinigames()
{
    for (int i = 0; i < m_numberOfMinigames; ++i)
    {
        m_queuedMinigameModes.push(new BattleRoyaleMinigameMode());
    }
}

//-----------------------------------------------------------------------------------
//PLAYER JOIN/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializePlayerJoinState()
{
    m_readyText[0] = new Sprite("ReadyText", PLAYER_LAYER);
    m_readyText[1] = new Sprite("ReadyText", PLAYER_LAYER);
    m_readyText[2] = new Sprite("ReadyText", PLAYER_LAYER);
    m_readyText[3] = new Sprite("ReadyText", PLAYER_LAYER);
    m_readyText[0]->m_position = Vector2(-1.0f, 1.0f);
    m_readyText[1]->m_position = Vector2(1.0f, 1.0f);
    m_readyText[2]->m_position = Vector2(-1.0f, -1.0f);
    m_readyText[3]->m_position = Vector2(1.0f, -1.0f);
    m_numberOfPlayers = 0;
    m_hasKeyboardPlayer = false;
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupPlayerJoinState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupPlayerJoinState(unsigned int)
{
    delete m_readyText[0];
    delete m_readyText[1];
    delete m_readyText[2];
    delete m_readyText[3];
    m_readyText[0] = nullptr;
    m_readyText[1] = nullptr;
    m_readyText[2] = nullptr;
    m_readyText[3] = nullptr;
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdatePlayerJoin(float)
{
    //If someone presses their button a second time, we know all players are in and we're ready to start.
    for (PlayerPilot* pilot : m_playerPilots)
    {
        if (pilot->m_inputMap.WasJustPressed("Accept"))
        {
            SetGameState(ASSEMBLY_GET_READY);
            InitializeAssemblyGetReadyState();
            return;
        }        
    }

    if (!m_hasKeyboardPlayer && m_numberOfPlayers < 4 && (InputSystem::instance->WasKeyJustPressed(' ') || InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER)))
    {
        m_readyText[m_numberOfPlayers]->m_tintColor = RGBA::GREEN;
        PlayerPilot* pilot = new PlayerPilot(m_numberOfPlayers++);
        m_playerPilots.push_back(pilot);
        InitializeKeyMappingsForPlayer(pilot);
    }

    for (int i = 0; i < 4; ++i)
    {
        XInputController* controller = InputSystem::instance->m_controllers[i];
        if (controller->IsConnected() && m_numberOfPlayers < 4 && controller->JustPressed(XboxButton::START))
        {
            m_readyText[m_numberOfPlayers]->m_tintColor = RGBA::GREEN;
            PlayerPilot* pilot = new PlayerPilot(m_numberOfPlayers++);
            pilot->m_controllerIndex = i;
            m_playerPilots.push_back(pilot);
            InitializeKeyMappingsForPlayer(pilot);
        }
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderPlayerJoin() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::GBBLACK);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//ASSEMBLY GET READY/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeAssemblyGetReadyState()
{
    m_getReadyBackground = new Sprite("AssemblyGetReady", PLAYER_LAYER);
    m_getReadyBackground->m_scale = Vector2(1.75f);
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupAssemblyGetReadyState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupAssemblyGetReadyState(unsigned int)
{
    delete m_getReadyBackground;
    m_getReadyBackground = nullptr;
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateAssemblyGetReady(float)
{
    if (g_secondsInState < TIME_BEFORE_PLAYERS_CAN_ADVANCE_UI)
    {
        return;
    }
    for (PlayerPilot* pilot : m_playerPilots)
    {
        if (pilot->m_inputMap.WasJustPressed("Accept"))
        {
            SetGameState(ASSEMBLY_PLAYING);
            InitializeAssemblyPlayingState();
        }

    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderAssemblyGetReady() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::FOREST_GREEN);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//ASSEMBLY PLAYING/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeAssemblyPlayingState()
{
    m_currentGameMode = static_cast<GameMode*>(new AssemblyMode());
    m_currentGameMode->Initialize();
    SpriteGameRenderer::instance->SetSplitscreen(m_playerPilots.size());
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupAssemblyPlayingState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupAssemblyPlayingState(unsigned int)
{
    SpriteGameRenderer::instance->SetCameraPosition(Vector2::ZERO);
    SpriteGameRenderer::instance->SetSplitscreen(1);
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateAssemblyPlaying(float deltaSeconds)
{
    m_currentGameMode->Update(deltaSeconds);
    if (!m_currentGameMode->m_isPlaying)
    {
        SetGameState(ASSEMBLY_RESULTS);
        TheGame::instance->InitializeAssemblyResultsState();
        m_currentGameMode->CleanUp();
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderAssemblyPlaying() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::FEEDFACE);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//ASSEMBLY RESULTS/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeAssemblyResultsState()
{
    m_currentGameMode->SetBackground("AssemblyResults", Vector2(1.75f));
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupAssemblyResultsState);
    for (unsigned int i = 0; i < TheGame::instance->m_players.size(); ++i)
    {
        PlayerShip* ship = TheGame::instance->m_players[i];
        ship->LockMovement();
        float xMultiplier = i % 2 == 0 ? -1.0f : 1.0f;
        float yMultiplier = i >= 2 ? -1.0f : 1.0f;
        ship->SetPosition(Vector2(3.0f * xMultiplier, 3.0f * yMultiplier));
    }
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupAssemblyResultsState(unsigned int)
{
    EnqueueMinigames();
    for (PlayerShip* ship : TheGame::instance->m_players)
    {
        ship->UnlockMovement();
        ship->m_isDead = false;
        ship->Heal(999999999.0f);
    }
    delete m_currentGameMode;
    m_currentGameMode = m_queuedMinigameModes.front();
    m_queuedMinigameModes.pop();
    SpriteGameRenderer::instance->SetCameraPosition(Vector2::ZERO);
    SpriteGameRenderer::instance->SetSplitscreen(1);
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateAssemblyResults(float deltaSeconds)
{
    for (PlayerShip* ship : TheGame::instance->m_players)
    {
        ship->Update(deltaSeconds);
    }
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER))
    {
        SetGameState(MINIGAME_GET_READY);
        InitializeMinigameGetReadyState();
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderAssemblyResults() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::GBLIGHTGREEN);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//MINIGAME GET READY/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeMinigameGetReadyState()
{
    m_getReadyBackground = new Sprite("BattleRoyaleGetReady", PLAYER_LAYER);
    m_getReadyBackground->m_scale = Vector2(1.75f);
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupMinigameGetReadyState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupMinigameGetReadyState(unsigned int)
{
    delete m_getReadyBackground;
    m_getReadyBackground = nullptr;
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateMinigameGetReady(float)
{
    if (g_secondsInState < TIME_BEFORE_PLAYERS_CAN_ADVANCE_UI)
    {
        return;
    }
    for (PlayerPilot* pilot : m_playerPilots)
    {
        if (pilot->m_inputMap.WasJustPressed("Accept"))
        {
            SetGameState(MINIGAME_PLAYING);
            InitializeMinigamePlayingState();
        }
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderMinigameGetReady() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::CORNFLOWER_BLUE);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//MINIGAME PLAYING/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeMinigamePlayingState()
{
    m_currentGameMode->Initialize();
    SpriteGameRenderer::instance->SetSplitscreen(m_playerPilots.size());
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupMinigamePlayingState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupMinigamePlayingState(unsigned int)
{
    SpriteGameRenderer::instance->SetCameraPosition(Vector2::ZERO);
    SpriteGameRenderer::instance->SetSplitscreen(1);
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateMinigamePlaying(float deltaSeconds)
{
    m_currentGameMode->Update(deltaSeconds);
    if (!m_currentGameMode->m_isPlaying)
    {
        SetGameState(MINIGAME_RESULTS);
        TheGame::instance->InitializeMinigameResultsState();
        m_currentGameMode->CleanUp();
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderMinigamePlaying() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::FEEDFACE);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//MINIGAME RESULTS/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeMinigameResultsState()
{
    SpriteGameRenderer::instance->SetCameraPosition(Vector2::ZERO);
    SpriteGameRenderer::instance->SetSplitscreen(1);
    m_currentGameMode->SetBackground("MinigameResults", Vector2(1.75f));
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupMinigameResultsState);
    for (unsigned int i = 0; i < TheGame::instance->m_players.size(); ++i)
    {
        PlayerShip* ship = TheGame::instance->m_players[i];
        ship->LockMovement();
        float xMultiplier = i % 2 == 0 ? -1.0f : 1.0f;
        float yMultiplier = i >= 2 ? -1.0f : 1.0f;
        ship->SetPosition(Vector2(3.0f * xMultiplier, 3.0f * yMultiplier));
    }
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupMinigameResultsState(unsigned int)
{
    for (PlayerShip* ship : TheGame::instance->m_players)
    {
        ship->UnlockMovement();
        ship->m_isDead = false;
        ship->Heal(999999999.0f);
    }
    delete m_currentGameMode;
    if (m_queuedMinigameModes.size() > 0)
    {
        m_currentGameMode = m_queuedMinigameModes.front();
        m_queuedMinigameModes.pop();
    }
    else
    {
        m_currentGameMode = nullptr;
    }
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateMinigameResults(float deltaSeconds)
{
    for (PlayerShip* ship : TheGame::instance->m_players)
    {
        ship->Update(deltaSeconds);
    }
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER))
    {
        if (m_queuedMinigameModes.size() > 0)
        {
            SetGameState(MINIGAME_GET_READY);
            TheGame::instance->InitializeMinigameGetReadyState();
        }
        else
        {
            SetGameState(GAME_RESULTS_SCREEN);
            TheGame::instance->InitializeGameOverState();
        }
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderMinigameResults() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::GBLIGHTGREEN);
    SpriteGameRenderer::instance->Render();
}


//-----------------------------------------------------------------------------------
//GAME OVER/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeGameOverState()
{
    m_gameOverText = new Sprite("GameOverText", PLAYER_LAYER);
    m_gameOverText->m_scale = Vector2(10.0f, 10.0f);
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupGameOverState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupGameOverState(unsigned int)
{
    delete m_gameOverText;
    for (PlayerShip* ship : m_players)
    {
        delete ship;
    }
    m_players.clear();
    AudioSystem::instance->PlaySound(SFX_UI_ADVANCE);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateGameOver(float )
{
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER))
    {
        SetGameState(MAIN_MENU);
        TheGame::instance->InitializeMainMenuState();
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderGameOver() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::DISEASED);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
void TheGame::InitializeKeyMappingsForPlayer(PlayerPilot* playerPilot)
{
    if (playerPilot->m_controllerIndex == -1)
    {
        KeyboardInputDevice* keyboard = InputSystem::instance->m_keyboardDevice;
        MouseInputDevice* mouse = InputSystem::instance->m_mouseDevice;
        //KEYBOARD & MOUSE INPUT
        playerPilot->m_inputMap.MapInputAxis("Up", keyboard->FindValue('W'), keyboard->FindValue('S'));
        playerPilot->m_inputMap.MapInputAxis("Right", keyboard->FindValue('D'), keyboard->FindValue('A'));
        playerPilot->m_inputMap.MapInputAxis("ShootUp", &mouse->m_deltaPosition.m_yAxis);
        playerPilot->m_inputMap.MapInputAxis("ShootRight", &mouse->m_deltaPosition.m_xAxis);
        playerPilot->m_inputMap.MapInputValue("Suicide", keyboard->FindValue('K'));
        playerPilot->m_inputMap.MapInputValue("Shoot", keyboard->FindValue(' '));
        playerPilot->m_inputMap.MapInputValue("Shoot", mouse->FindButtonValue(InputSystem::MouseButton::LEFT_MOUSE_BUTTON));
        playerPilot->m_inputMap.MapInputValue("Accept", keyboard->FindValue(InputSystem::ExtraKeys::ENTER));
        playerPilot->m_inputMap.MapInputValue("Accept", keyboard->FindValue(' '));
        playerPilot->m_inputMap.MapInputValue("Respawn", keyboard->FindValue(' '));
        playerPilot->m_inputMap.MapInputValue("Respawn", keyboard->FindValue('R'));
    }
    else
    {
        XInputDevice* controller = InputSystem::instance->m_xInputDevices[playerPilot->m_controllerIndex];
        //CONTROLLER INPUT
        playerPilot->m_inputMap.MapInputAxis("Up")->AddMapping(&controller->GetLeftStick()->m_yAxis);
        playerPilot->m_inputMap.MapInputAxis("Right")->AddMapping(&controller->GetLeftStick()->m_xAxis);
        playerPilot->m_inputMap.MapInputAxis("ShootUp")->AddMapping(&controller->GetRightStick()->m_yAxis);
        playerPilot->m_inputMap.MapInputAxis("ShootRight")->AddMapping(&controller->GetRightStick()->m_xAxis);
        playerPilot->m_inputMap.MapInputValue("Suicide", controller->FindButton(XboxButton::BACK));
        playerPilot->m_inputMap.MapInputValue("Shoot", ChordResolutionMode::RESOLVE_MAXS_ABSOLUTE)->m_deadzoneValue = XInputController::INNER_DEADZONE;
        playerPilot->m_inputMap.MapInputValue("Shoot", controller->GetRightTrigger());
        playerPilot->m_inputMap.MapInputValue("Shoot", controller->GetRightStickMagnitude());
        playerPilot->m_inputMap.MapInputValue("Shoot", controller->FindButton(XboxButton::A));
        playerPilot->m_inputMap.MapInputValue("Accept", controller->FindButton(XboxButton::A));
        playerPilot->m_inputMap.MapInputValue("Accept", controller->FindButton(XboxButton::START));
        playerPilot->m_inputMap.MapInputValue("Respawn", controller->FindButton(XboxButton::START));
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RegisterSprites()
{
    //Backgrounds
    ResourceDatabase::instance->RegisterSprite("Twah", "Data\\Images\\Twah.png");
    ResourceDatabase::instance->RegisterSprite("DefaultBackground", "Data\\Images\\Backgrounds\\Nebula.jpg");
    //ResourceDatabase::instance->RegisterSprite("DefaultBackground", "Data\\Images\\Backgrounds\\DefaultBackground.jpg");
    ResourceDatabase::instance->RegisterSprite("BattleBackground", "Data\\Images\\Backgrounds\\Orange-space.jpg");
    ResourceDatabase::instance->RegisterSprite("AssemblyResults", "Data\\Images\\assemblyResultsMockup.png");
    ResourceDatabase::instance->RegisterSprite("MinigameResults", "Data\\Images\\minigameResultsMockup.png");
    ResourceDatabase::instance->RegisterSprite("AssemblyGetReady", "Data\\Images\\assemblyGetReadyMockup.png");
    ResourceDatabase::instance->RegisterSprite("BattleRoyaleGetReady", "Data\\Images\\battleRoyaleGetReadyMockup.png");
    ResourceDatabase::instance->RegisterSprite("ReadyText", "Data\\Images\\ready.png");

    //Entities
    ResourceDatabase::instance->RegisterSprite("Laser", "Data\\Images\\Lasers\\laserColorless10.png");
    ResourceDatabase::instance->RegisterSprite("Pico", "Data\\Images\\Pico.png");
    ResourceDatabase::instance->RegisterSprite("PlayerShip", "Data\\Images\\garbageRecolorableShip.png");
    ResourceDatabase::instance->RegisterSprite("Shield", "Data\\Images\\Shield.png");
    ResourceDatabase::instance->RegisterSprite("TitleText", "Data\\Images\\Title.png");
    ResourceDatabase::instance->RegisterSprite("GameOverText", "Data\\Images\\GameOver.png");
    ResourceDatabase::instance->RegisterSprite("ItemBox", "Data\\Images\\ItemBox.png");
    ResourceDatabase::instance->RegisterSprite("GreenEnemy", "Data\\Images\\Enemies\\enemyGreen1.png");
    ResourceDatabase::instance->RegisterSprite("Asteroid", "Data\\Images\\Props\\asteroid01.png");
    ResourceDatabase::instance->RegisterSprite("Invalid", "Data\\Images\\invalidSpriteResource.png");

    //Pickups
    ResourceDatabase::instance->RegisterSprite("TopSpeed", "Data\\Images\\Pickups\\speed.png");
    ResourceDatabase::instance->RegisterSprite("Acceleration", "Data\\Images\\Pickups\\boost.png");
    ResourceDatabase::instance->RegisterSprite("Handling", "Data\\Images\\Pickups\\handling.png");
    ResourceDatabase::instance->RegisterSprite("Braking", "Data\\Images\\Pickups\\braking.png");
    ResourceDatabase::instance->RegisterSprite("Damage", "Data\\Images\\Pickups\\power.png");
    ResourceDatabase::instance->RegisterSprite("ShieldDisruption", "Data\\Images\\invalidSpriteResource.png");
    ResourceDatabase::instance->RegisterSprite("ShieldPenetration", "Data\\Images\\invalidSpriteResource.png");
    ResourceDatabase::instance->RegisterSprite("RateOfFire", "Data\\Images\\Pickups\\fireRate.png");
    ResourceDatabase::instance->RegisterSprite("Hp", "Data\\Images\\Pickups\\hp.png");
    ResourceDatabase::instance->RegisterSprite("ShieldCapacity", "Data\\Images\\invalidSpriteResource.png");
    ResourceDatabase::instance->RegisterSprite("ShieldRegen", "Data\\Images\\invalidSpriteResource.png");
    ResourceDatabase::instance->RegisterSprite("ShotDeflection", "Data\\Images\\invalidSpriteResource.png");

    //Particles
    ResourceDatabase::instance->RegisterSprite("Placeholder", "Data\\Images\\Particles\\placeholder.png");
    ResourceDatabase::instance->RegisterSprite("Yellow4Star", "Data\\Images\\Particles\\particleYellow_7.png");
    ResourceDatabase::instance->RegisterSprite("YellowCircle", "Data\\Images\\Particles\\particleYellow_8.png");
    ResourceDatabase::instance->RegisterSprite("YellowBeam", "Data\\Images\\Particles\\particleYellow_9.png");
    ResourceDatabase::instance->EditSpriteResource("YellowBeam")->m_pivotPoint.y = 0.0f;

}

//-----------------------------------------------------------------------------------
void TheGame::RegisterParticleEffects()
{
    const float DEATH_ANIMATION_LENGTH = 1.5f;
    const float POWER_UP_PICKUP_ANIMATION_LENGTH = 0.3f;
    const float DEAD_SHIP_LINGERING_SECONDS = 15.0f;
    const float MUZZLE_FLASH_ANIMATION_LENGTH = 0.2f;
    const float CRATE_DESTRUCTION_ANIMATION_LENGTH = 0.6f;

    //EMITTERS/////////////////////////////////////////////////////////////////////
    ParticleEmitterDefinition* yellowStars = new ParticleEmitterDefinition(ResourceDatabase::instance->GetSpriteResource("Yellow4Star"));
    yellowStars->m_fadeoutEnabled = true;
    yellowStars->m_initialNumParticlesSpawn = Range<unsigned int>(10,15);
    yellowStars->m_initialScalePerParticle = Range<Vector2>(Vector2(0.2f), Vector2(0.4f));
    yellowStars->m_initialVelocity = Vector2::ZERO;
    yellowStars->m_lifetimePerParticle = 0.2f;
    yellowStars->m_particlesPerSecond = 40.0f;
    yellowStars->m_maxLifetime = DEATH_ANIMATION_LENGTH;
    yellowStars->m_spawnRadius = Range<float>(0.4f, 0.6f);
    yellowStars->m_scaleRateOfChangePerSecond = Vector2(1.3f);

    ParticleEmitterDefinition* yellowExplosionOrb = new ParticleEmitterDefinition(ResourceDatabase::instance->GetSpriteResource("YellowCircle"));
    yellowExplosionOrb->m_fadeoutEnabled = true;
    yellowExplosionOrb->m_initialNumParticlesSpawn = 1;
    yellowExplosionOrb->m_initialScalePerParticle = Range<Vector2>(Vector2(0.2f), Vector2(0.4f));
    yellowExplosionOrb->m_initialVelocity = Vector2::ZERO;
    yellowExplosionOrb->m_lifetimePerParticle = DEATH_ANIMATION_LENGTH;
    yellowExplosionOrb->m_particlesPerSecond = 0.0f;
    yellowExplosionOrb->m_scaleRateOfChangePerSecond = Vector2(1.3f);

    ParticleEmitterDefinition* yellowBeams = new ParticleEmitterDefinition(ResourceDatabase::instance->GetSpriteResource("YellowBeam"));
    yellowBeams->m_fadeoutEnabled = true;
    yellowBeams->m_initialNumParticlesSpawn = Range<unsigned int>(5, 10);
    yellowBeams->m_initialScalePerParticle = Range<Vector2>(Vector2(0.2f, 0.2f), Vector2(0.4f, 0.4f));
    yellowBeams->m_initialVelocity = Vector2::ZERO;
    yellowBeams->m_lifetimePerParticle = DEATH_ANIMATION_LENGTH;
    yellowBeams->m_particlesPerSecond = 0.0f;
    yellowBeams->m_scaleRateOfChangePerSecond = Vector2(0.0f, 2.0f);
    yellowBeams->m_initialRotationDegrees = Range<float>(0.0f, 360.0f);

    ParticleEmitterDefinition* powerupPickup = new ParticleEmitterDefinition(ResourceDatabase::instance->GetSpriteResource("Placeholder"));
    powerupPickup->m_fadeoutEnabled = true;
    powerupPickup->m_initialNumParticlesSpawn = Range<unsigned int>(5, 15);
    powerupPickup->m_initialScalePerParticle = Range<Vector2>(Vector2(0.2f), Vector2(0.4f));
    powerupPickup->m_initialVelocity = Vector2::UNIT_Y;
    powerupPickup->m_lifetimePerParticle = 0.5f;
    powerupPickup->m_particlesPerSecond = 0.0f;
    powerupPickup->m_maxLifetime = POWER_UP_PICKUP_ANIMATION_LENGTH;
    powerupPickup->m_spawnRadius = Range<float>(0.4f, 0.6f);
    powerupPickup->m_scaleRateOfChangePerSecond = Vector2(1.3f);

    ParticleEmitterDefinition* muzzleFlash = new ParticleEmitterDefinition(ResourceDatabase::instance->GetSpriteResource("Placeholder"));
    muzzleFlash->m_fadeoutEnabled = true;
    muzzleFlash->m_initialNumParticlesSpawn = Range<unsigned int>(2,4);
    muzzleFlash->m_initialScalePerParticle = Range<Vector2>(Vector2(0.2f), Vector2(0.4f));
    muzzleFlash->m_initialVelocity = Vector2::UNIT_Y;
    muzzleFlash->m_lifetimePerParticle = 0.2f;
    muzzleFlash->m_particlesPerSecond = 0.0f;
    muzzleFlash->m_maxLifetime = MUZZLE_FLASH_ANIMATION_LENGTH;
    muzzleFlash->m_scaleRateOfChangePerSecond = Vector2(0.3f);

    ParticleEmitterDefinition* crateDestroyed = new ParticleEmitterDefinition(ResourceDatabase::instance->GetSpriteResource("Placeholder"));
    crateDestroyed->m_fadeoutEnabled = true;
    crateDestroyed->m_initialNumParticlesSpawn = Range<unsigned int>(5, 15);
    crateDestroyed->m_initialScalePerParticle = Range<Vector2>(Vector2(0.2f), Vector2(0.4f));
    crateDestroyed->m_lifetimePerParticle = 0.5f;
    crateDestroyed->m_particlesPerSecond = 0.0f;
    crateDestroyed->m_maxLifetime = CRATE_DESTRUCTION_ANIMATION_LENGTH;
    crateDestroyed->m_spawnRadius = Range<float>(0.4f, 0.6f);
    crateDestroyed->m_scaleRateOfChangePerSecond = Vector2(1.3f);
    crateDestroyed->m_initialRotationDegrees = Range<float>(0.0f, 360.0f);

    //SYSTEMS/////////////////////////////////////////////////////////////////////
    ParticleSystemDefinition* deathParticleSystem = ResourceDatabase::instance->RegisterParticleSystem("Death", ONE_SHOT);
    deathParticleSystem->AddEmitter(yellowStars);
    deathParticleSystem->AddEmitter(yellowExplosionOrb);
    deathParticleSystem->AddEmitter(yellowBeams);

    ParticleSystemDefinition* powerupPickupParticleSystem = ResourceDatabase::instance->RegisterParticleSystem("PowerupPickup", ONE_SHOT);
    powerupPickupParticleSystem->AddEmitter(powerupPickup);

    ParticleSystemDefinition* muzzleFlashParticleSystem = ResourceDatabase::instance->RegisterParticleSystem("MuzzleFlash", ONE_SHOT);
    muzzleFlashParticleSystem->AddEmitter(muzzleFlash);

    ParticleSystemDefinition* crateDestroyedParticleSystem = ResourceDatabase::instance->RegisterParticleSystem("CrateDestroyed", ONE_SHOT);
    crateDestroyedParticleSystem->AddEmitter(crateDestroyed);
}
