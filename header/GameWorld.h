#pragma once

class Mario;

#include "Drawable.h"
#include "GameState.h"
#include "Map.h"
#include "ResourceManager.h"
#include "TitleScreen.h"
//#include "Mario.h"
#include "raylib.h"
#include <iostream>

class GameWorld : public virtual Drawable {

    TitleScreen* titleScreen=nullptr;

    //Mario mario;
    Map map;
    Camera2D* camera;
    //bool showControls;
    //GameState stateBeforePause;
    int remainingTimePointCount;

    // bool pauseMusic;
    // bool pauseMarioUpdate;
    // bool showOverlayOnPause;

    // bool irisOutFinished;
    // float irisOutTime;
    // float irisOutAcum;

public:

    // static bool immortalMario;
    static GameState state;
    static float gravity;

    /**
     * @brief Construct a new GameWorld object.
     */
    GameWorld();

    /**
     * @brief Destroy the GameWorld object.
     */
    ~GameWorld() override;

    /**
     * @brief Reads user input and updates the state of the game.
     */
    void inputAndUpdate();

    /**
     * @brief Draws the state of the game.
     */
    void draw() override;

    /**
     * @brief Load game resources like images, textures, sounds, fonts, shaders,
     * etc.
     * Should be called inside the constructor.
     */
    static void loadResources();

    /**
     * @brief Unload the once loaded game resources.
     * Should be called inside the destructor.
     */
    static void unloadResources();

    void setCamera(Camera2D* camera);
    Camera2D* getCamera() const;

    void resetMap();
    void resetGame();
    void nextMap();
    void pauseGame(bool playPauseSFX, bool pauseMusic, bool showOverlay, bool pauseMarioUpdate);
    void unpauseGame();

    // bool isPauseMusicOnPause() const;
    // bool isShowOverlayOnPause() const;

};