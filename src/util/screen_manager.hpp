#pragma once

#include "../systems/render_system.hpp"

class RenderSystem;
class WorldSystem;
class UpgradeSystem;

enum class ScreenType {
    StartScreen,
    PlayScreen,
    PausedScreen,
    TutorialScreen,
    TutorialPausedScreen,
    CreditsScreen,
    UpgradeScreen,
    TutorialUpgradeScreen,
    DeathScreen
};

class ScreenManager {
public:
    ScreenManager(RenderSystem* renderSystem, WorldSystem* worldSystem, UpgradeSystem* upgrade_system);
    void setScreen(ScreenType newScreen);
    void renderScreen();
    void handleInput(GLFWwindow* window);
    ScreenType getCurrentScreen() const;

    void endTutorial();

    void restartGame();

    // Getters
    std::vector<std::string> getMenuItems();
    int getSelectedIndex();
    int getCurrentTutorialStep();

private:
    ScreenType currentScreen;
    RenderSystem* renderer;
    WorldSystem* world;
    UpgradeSystem* upgrade_system;

    // For Menu
    std::vector<std::string> menuItems = {"Play", "Tutorial", "Upgrade", "Credits"};
    int selectedIndex = 0;
    const float menuX = 100.0f;
    const float menuYStart = 150.0f;
    const float menuHeight = 50.0f;
    const float menuWidth = 300.0f;

    void handleStartMenuSelection(int selectedIndex);
    void handlePausedMenuSelection(int selectedIndex);

    // For Tutorial
    int tutorialStep = 0;
    vec2 startPos;
    bool isTrackingMovement = false;
    bool rightMousePressed = false;
};