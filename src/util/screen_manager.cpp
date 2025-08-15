#include "screen_manager.hpp"
#include <iostream>
#include "../tinyECS/registry.hpp"
#include "../systems/world_system.hpp"

ScreenManager::ScreenManager(RenderSystem* renderSystem, WorldSystem* worldSystem, UpgradeSystem* upgrade_system) :
    currentScreen(ScreenType::StartScreen), renderer(renderSystem), world(worldSystem), upgrade_system(upgrade_system){}

void ScreenManager::setScreen(ScreenType newScreen) {
    currentScreen = newScreen;
    
    if (currentScreen == ScreenType::StartScreen) {
        menuItems = {"Play", "Tutorial", "Upgrade", "Credits"};
        selectedIndex = 0;
    }

    if (currentScreen == ScreenType::PausedScreen) {
        menuItems = {"Resume", "Save and Quit Game"};
        selectedIndex = 0;
    }

    if (currentScreen == ScreenType::TutorialPausedScreen) {
        menuItems = {"Press Enter to resume the tutorial"};
        selectedIndex = 0;
    }

    if (currentScreen == ScreenType::DeathScreen) {
        menuItems = { "Press Enter to return to main menu" };
        selectedIndex = 0;
    }
}

void ScreenManager::renderScreen() {
    switch(currentScreen) {
        case ScreenType::StartScreen:
            renderer->drawStartScreen(*this);
            world->change_background_music(*this);
            break;
        case ScreenType::PlayScreen:
            renderer->draw();
            world->change_background_music(*this);
            break;
        case ScreenType::PausedScreen:
            renderer->drawPausedScreen(*this);
            world->change_background_music(*this);
            break;
        case ScreenType::TutorialScreen:
            renderer->draw();
            world->change_background_music(*this);
            break;
        case ScreenType::TutorialPausedScreen:
            renderer->drawPausedScreen(*this);
            world->change_background_music(*this);
            break;
        case ScreenType::UpgradeScreen:
            renderer->drawUpgradeScreen(*this, *upgrade_system);
            break;
        case ScreenType::TutorialUpgradeScreen:
            renderer->drawUpgradeScreen(*this, *upgrade_system);
            break;
        case ScreenType::CreditsScreen:
            renderer->drawCreditsScreen(*this);
            world->change_background_music(*this);
            break;
        case ScreenType::DeathScreen:
            renderer->drawDeathScreen(*this);
            break;
    }
}

void ScreenManager::handleInput(GLFWwindow* window) {
    static bool upKeyPressed = false;
    static bool downKeyPressed = false;
    static bool enterKeyPressed = false;
    static bool pKeyPressed = false;
    static bool iKeyPressed = false;
    static bool mousePressed = false;

    Entity screen_state_entity = renderer->get_screen_state_entity();
    ScreenState& screen_state = registry.screenStates.get(screen_state_entity);

    // Navigating menus in StartScreen and PausedScreen
    if (currentScreen == ScreenType::StartScreen || currentScreen == ScreenType::PausedScreen) {        
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseY = WINDOW_HEIGHT_PX - mouseY;

        for (int i = 0; i < menuItems.size(); ++i) {

            glm::vec2 menuItemPosition = {100, 300 + i * -50.f};
            
            bool isHovered = (mouseX >= menuItemPosition.x && mouseX <= menuItemPosition.x + 200.f &&
                                mouseY >= menuItemPosition.y && mouseY <= menuItemPosition.y + 48.f);
    
            if (isHovered && selectedIndex != i) {
                selectedIndex = i;
                world->play_sound_effect("menu_change");
            }

            if (isHovered && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mousePressed) {
                world->play_sound_effect("enter");
                if (currentScreen == ScreenType::StartScreen) handleStartMenuSelection(selectedIndex);
                else if (currentScreen == ScreenType::PausedScreen) handlePausedMenuSelection(selectedIndex);
                mousePressed = true;
            } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
                mousePressed = false;
            }
        }
    }

    // Pausing Game
    if (currentScreen == ScreenType::PlayScreen && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed) {
        setScreen(ScreenType::PausedScreen);
        screen_state.is_paused = !screen_state.is_paused;
        world->play_sound_effect("enter");
        pKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pKeyPressed = false;
    }

    // Pausing Tutorial
    if (currentScreen == ScreenType::TutorialPausedScreen && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterKeyPressed) {
        tutorialStep = 14;
        screen_state.is_paused = !screen_state.is_paused;
        setScreen(ScreenType::TutorialScreen);
        world->play_sound_effect("enter");
        enterKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        enterKeyPressed = false;
    }

    // Tutorial Upgrade
    if (currentScreen == ScreenType::TutorialUpgradeScreen && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterKeyPressed) {
        tutorialStep = 15;
        setScreen(ScreenType::TutorialScreen);
        world->play_sound_effect("enter");
        enterKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        enterKeyPressed = false;
    }

    // Inventory
    if ((currentScreen == ScreenType::PlayScreen || currentScreen == ScreenType::TutorialScreen )&& glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !iKeyPressed) {
        iKeyPressed = true;
        screen_state.isInventoryClosed = !screen_state.isInventoryClosed;
    } else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) {
        iKeyPressed = false;
    }

    // Back 'button' for CreditsScreen
    if (currentScreen == ScreenType::CreditsScreen) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseY = WINDOW_HEIGHT_PX - mouseY;

        glm::vec2 backPos = glm::vec2(797.5, 100); // same as renderText position

        if (mouseX >= backPos.x && mouseX <= backPos.x + 100 && mouseY >= backPos.y && mouseY <= backPos.y + 50.f) {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mousePressed) {
                setScreen(ScreenType::StartScreen);
                world->play_sound_effect("enter");
                mousePressed = true;
            } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
                mousePressed = false;
            }
        }
    }

    // Death Screen
    if (currentScreen == ScreenType::DeathScreen && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterKeyPressed) {
        setScreen(ScreenType::StartScreen);
        restartGame();
        screen_state.is_death = false;
        enterKeyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        enterKeyPressed = false;
    }

    // [5] Improved Gameplay: Gameplay Tutorial
    // Game Tutorial
    if (currentScreen == ScreenType::TutorialScreen) {

        Entity player_entity = registry.players.entities[0];
        Player& player = registry.players.get(player_entity);
        vec2 player_position = registry.positions.get(player_entity).position;

        float enemy_health = registry.enemies.entities.empty() ? 0.0f : registry.livings.get(registry.enemies.entities[0]).current_health;

        Entity screen_state_entity = renderer->get_screen_state_entity();
        ScreenState& screen_state = registry.screenStates.get(screen_state_entity);
        screen_state.tutorialStep = tutorialStep;

        switch (tutorialStep)
        {
        case 0:
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                if (!isTrackingMovement) {
                    startPos = player_position;
                    isTrackingMovement = true;
                }
                if (glm::distance(player_position, startPos) >= 50.0f) {
                    tutorialStep++;
                    isTrackingMovement = false;
                }
            }
            break;
        case 1:
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                if (!isTrackingMovement) {
                    startPos = player_position;
                    isTrackingMovement = true;
                }
                if (glm::distance(player_position, startPos) >= 50.0f) {
                    tutorialStep++;
                    isTrackingMovement = false;
                }
            }
            break;
        case 2:
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                if (!isTrackingMovement) {
                    startPos = player_position;
                    isTrackingMovement = true;
                }
                if (glm::distance(player_position, startPos) >= 50.0f) {
                    tutorialStep++;
                    isTrackingMovement = false;
                }
            }
            break;
        case 3:
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                if (!isTrackingMovement) {
                    startPos = player_position;
                    isTrackingMovement = true;
                }
                if (glm::distance(player_position, startPos) >= 50.0f) {
                    tutorialStep++;
                    isTrackingMovement = false;
                }
            }
            break;
        case 4:
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                if (!isTrackingMovement) {
                    startPos = player_position;
                    isTrackingMovement = true;
                }
                if (glm::distance(player_position, startPos) >= 30.0f) {
                    tutorialStep++;
                    isTrackingMovement = false;
                }
            }
            break;
        case 5:
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                tutorialStep++;
                world->spawn_enemy_pack();
            }
            break;
        case 6:
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
                if (enemy_health <= 0) {
                    tutorialStep++;
                }
            }
            break;
        case 7:
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                if (registry.parries.entities.size() != 0){
                    tutorialStep++;
                }
            }
            break;
        case 8:
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                tutorialStep++;
                rightMousePressed = true;
            }
            break;
        case 9:
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !rightMousePressed) {
                tutorialStep++;
                world->spawn_enemy_pack();
            }

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && rightMousePressed) {
                rightMousePressed = false;
            }
            break;
        case 10:
            if (enemy_health <= 0) {
                tutorialStep++;
            }
            break;
        case 11:
            if (registry.players.get(registry.players.entities[0]).inventory.size() == 1) {
                tutorialStep++;
            }
            break;
        case 12:
            if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
                tutorialStep++;
            }
            break;
        case 13:
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                setScreen(ScreenType::TutorialPausedScreen);
                screen_state.is_paused = !screen_state.is_paused;
            }
            break;
        case 14:
            setScreen(ScreenType::TutorialUpgradeScreen);
            break;
        case 15:
            // Player needs to walk to exit
            break;
        }

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) enterKeyPressed = false;
    }
}

void ScreenManager::handleStartMenuSelection(int selectedIndex) {
    Entity screen_state_entity = renderer->get_screen_state_entity();
    ScreenState& screen_state = registry.screenStates.get(screen_state_entity);
    
    switch (selectedIndex) {
        case 0: 
            screen_state.tutorialActive = false;
            setScreen(ScreenType::PlayScreen);
            break;
        case 1: 
            screen_state.tutorialActive = true;
            setScreen(ScreenType::TutorialScreen);
            break;
        case 2: 
            setScreen(ScreenType::UpgradeScreen);
            break;
        case 3: 
            setScreen(ScreenType::CreditsScreen);
            break;
    }
}

void ScreenManager::handlePausedMenuSelection(int selectedIndex) {
    Entity screen_state_entity = renderer->get_screen_state_entity();
    ScreenState& screen_state = registry.screenStates.get(screen_state_entity);

    switch (selectedIndex) {
        case 0:
            setScreen(ScreenType::PlayScreen);
            screen_state.is_paused = !screen_state.is_paused;
            break;
        case 1: 
            setScreen(ScreenType::StartScreen);
            screen_state.is_paused = !screen_state.is_paused;
            restartGame();
            break;
    }
}

void ScreenManager::restartGame() {
    Entity screen_state_entity = renderer->get_screen_state_entity();
    ScreenState& screen_state = registry.screenStates.get(screen_state_entity);
    screen_state.isInventoryClosed = true;

    world->restart_game();
}

// Getters
ScreenType ScreenManager::getCurrentScreen() const {
    return currentScreen;
}

std::vector<std::string> ScreenManager::getMenuItems() {
    return menuItems;
}

int ScreenManager::getSelectedIndex() {
    return selectedIndex;
}

int ScreenManager::getCurrentTutorialStep() {
    return tutorialStep;
}

void ScreenManager::endTutorial() {
    Entity screen_state_entity = renderer->get_screen_state_entity();
    ScreenState& screen_state = registry.screenStates.get(screen_state_entity);
    screen_state.tutorialStep = tutorialStep;

    setScreen(ScreenType::StartScreen);
    tutorialStep = 0;
    screen_state.tutorialActive = false;
    screen_state.isInventoryClosed = true;
}