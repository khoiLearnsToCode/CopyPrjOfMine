#include "TitleScreen.h"
#include "ResourceManager.h"

TitleScreen::TitleScreen() : Screen(),
    startButton("StartButton", {GetScreenWidth() - 330.0f, GetScreenHeight() - 110.0f} , 0.3f), 
    credit("Credit", {100.0f, GetScreenHeight() - 60.0f}, 0.3f, BLACK, GetFontDefault(), 40.0f),
    temp("Test", "longButton", {GetScreenWidth() / 2.0f - 100.0f, GetScreenHeight() / 2.0f}, 1.0f, WHITE, GetFontDefault(), 20.0f) {
    std::cout << "Screen size: " << GetScreenWidth() << " x " << GetScreenHeight() << std::endl;
    std::cout << "temp pos: " << (GetScreenWidth() / 2.0f - 100.0f) << ", " << (GetScreenHeight() / 2.0f) << std::endl;
}

void TitleScreen::draw() {
    DrawTexture(textures["title_screen"], 0, 0, WHITE);
    DrawTexture(textures["guiRayMarioLogo"], 10, 10, WHITE);
    startButton.draw();
    credit.draw();
    temp.draw();
}

ButtonTexture& TitleScreen::getStartButton() {
    return startButton;
}

ButtonText& TitleScreen::getCreditButton() {
    return credit;
}
