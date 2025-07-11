#include "Direction.h"
#include "GameState.h"
#include "GameWorld.h"
#include "Map.h"
#include "Mario.h"
#include "raylib.h"
#include "ResourceManager.h"
#include <iostream>
#include <utils.h>

// #include "FireFlower.h"
// #include "Mushroom.h"

Mario::Mario( Vector2 pos, Vector2 dim, Vector2 vel, Color color, float speedX, float maxSpeedX, float jumpSpeed, bool immortal ) :
	Sprite( pos, dim, vel, color, 0, 2),
	speedX( speedX ),
	maxSpeedX( maxSpeedX ),
	jumpSpeed( jumpSpeed ),
	dyingVelY( -600 ),
	immortal( immortal ),
	invulnerable( false ),
	ducking( false ),
	lookingUp( false ),
	running( false ),
	invincible( false ),
	frameTimeWalking( 0.1 ),
	frameTimeRunning( 0.5 ),
    activationWidth(0),
    lives(5),
    coins(0),
    yoshiCoins(0),
    points(0),
    maxTime(0),
    ellapsedTime(0.0f),
    type(MARIO_TYPE_SMALL),
    reservedPowerUp(MARIO_TYPE_SMALL),
    runningAcum(0),
    runningTime(0.5),
    drawRunningFrames(false),
    movingAcum(0),

    transitioningFrameTime(0.06),
    transitioningFrameAcum(0),
    transitionSteps(11),
    superToFlowerTransitionSteps(8),
    transitionCurrentFrame(0),
    transitionCurrentFramePos(0),

    invincibleTime(8),
    invincibleAcum(0),
    playerDownMusicStreamPlaying(false),
    gameOverMusicStreamPlaying(false),
    lastPos(pos),
    lastStateBeforeTransition(SPRITE_STATE_ACTIVE),
    gw(nullptr),
    map(nullptr) {

    setState(SPRITE_STATE_ON_GROUND);

    cpN.setColor(PINK);
    cpS.setColor(VIOLET);
    cpE.setColor(YELLOW);
    cpE1.setColor(YELLOW);
    cpW.setColor(LIME);
    cpW1.setColor(LIME);

}

Mario::~Mario() = default;

void Mario::update() {
    const float delta = GetFrameTime();

    // Mario is running if Left Ctrl is hold and he is moving (velocity is non-zero)
    running = (IsKeyDown(KEY_LEFT_CONTROL) && vel.x != 0.0f);

    if (running) {
        runningAcum += delta;
        if (runningAcum >= runningTime) {
            drawRunningFrames = true;
        }
    } else {
        runningAcum = 0;
        drawRunningFrames = false;
    }

    if (invincible) {
        invincibleAcum += delta;
        if (invincibleAcum >= invincibleTime) {
            invincible = false;
            invincibleAcum = 0;
        }
    }

    if (state != SPRITE_STATE_DYING && state != SPRITE_STATE_VICTORY && state != SPRITE_STATE_WAITING_TO_NEXT_MAP) {
        ellapsedTime += delta;
    }

    // If walking: speedX, if running but not in full animation: maxSpeedX, if running in full animation: maxSpeedX * 1.3f
    const float currentSpeedX = running ? (drawRunningFrames ? maxSpeedX * 1.3f : maxSpeedX) : speedX;

    const float currentFrameTime = running && state != SPRITE_STATE_DYING ? frameTimeRunning : frameTimeWalking;

	std::map<std::string, Sound>& sounds = ResourceManager::getInstance().getSounds(); 

    if (ellapsedTime >= maxTime &&
        state != SPRITE_STATE_DYING &&
        state != SPRITE_STATE_VICTORY &&
        state != SPRITE_STATE_WAITING_TO_NEXT_MAP) {
        state = SPRITE_STATE_DYING;
        playPlayerDownMusicStream();
		removeLives(1);
		changeToSmall();
        GameWorld::state = GAME_STATE_TIME_UP;
    }

    if (vel.x != 0 || state == SPRITE_STATE_DYING) {
		frameAcum += delta;
        if (frameAcum >= currentFrameTime) {
            frameAcum = 0;
            currentFrame++;
            currentFrame %= maxFrames;
        };
    }
    else {
        currentFrame = 0;
    }

    if (state == SPRITE_STATE_DYING) {
		pos.y += dyingVelY * delta;
		updateCollisionProbes();
        dyingVelY += GameWorld::gravity;
	}

    if (invulnerable) {
        invulnerableTimeAcum += delta;
		invulnerableBlink = !invulnerableBlink; // Toggle blink state
        if (invulnerableTimeAcum >= invulnerableTime) {
            invulnerableTimeAcum = 0;
            invulnerable = false;
            invulnerableBlink = false;
        }
    }

    if (state != SPRITE_STATE_DYING &&
        state != SPRITE_STATE_VICTORY &&
        state != SPRITE_STATE_WAITING_TO_NEXT_MAP) {

        if (state == SPRITE_STATE_TRANSITIONING_SMALL_TO_SUPER) {

            transitioningFrameAcum += delta;
            if (transitioningFrameAcum >= transitioningFrameTime) {
                transitioningFrameAcum = 0;
                transitionCurrentFramePos++;
                if (transitionCurrentFramePos <= transitionSteps) {
                    transitionCurrentFrame = transitionFrameOrder[transitionCurrentFramePos];
                }
                else {
                    transitionCurrentFramePos = 0;
                    state = lastStateBeforeTransition;
                    changeToSuper();
                    gw->unpauseGame();
                }
            }

        }
        else if (state == SPRITE_STATE_TRANSITIONING_SMALL_TO_FLOWER) {

            transitioningFrameAcum += delta;
            if (transitioningFrameAcum >= transitioningFrameTime) {
                transitioningFrameAcum = 0;
                transitionCurrentFramePos++;
                if (transitionCurrentFramePos <= transitionSteps) {
                    transitionCurrentFrame = transitionFrameOrder[transitionCurrentFramePos];
                }
                else {
                    transitionCurrentFramePos = 0;
                    state = lastStateBeforeTransition;
                    changeToFlower();
                    gw->unpauseGame();
                }
            }

        }
        else if (state == SPRITE_STATE_TRANSITIONING_SUPER_TO_FLOWER) {

            transitioningFrameAcum += delta;
            if (transitioningFrameAcum >= transitioningFrameTime) {
                transitioningFrameAcum = 0;
                transitionCurrentFramePos++;
                if (transitionCurrentFramePos <= superToFlowerTransitionSteps) {
                    transitionCurrentFrame = superToFlowerTransitionFrameOrder[transitionCurrentFramePos];
                }
                else {
                    transitionCurrentFramePos = 0;
                    state = lastStateBeforeTransition;
                    changeToFlower();
                    gw->unpauseGame();
                }
            }

        }
        else if (state == SPRITE_STATE_TRANSITIONING_SUPER_TO_SMALL ||
            state == SPRITE_STATE_TRANSITIONING_FLOWER_TO_SMALL) {

            transitioningFrameAcum += delta;
            if (transitioningFrameAcum >= transitioningFrameTime) {
                transitioningFrameAcum = 0;
                transitionCurrentFramePos++;
                if (transitionCurrentFramePos <= transitionSteps) {
                    transitionCurrentFrame = reverseTransitionFrameOrder[transitionCurrentFramePos];
                }
                else {
                    transitionCurrentFramePos = 0;
                    state = lastStateBeforeTransition;
                    changeToSmall();
                    // releaseReservedPowerUp();
                    gw->unpauseGame();
                }
            }

        }
        else {

            if (IsKeyDown(KEY_RIGHT)) {
                facingDirection = DIRECTION_RIGHT;
                movingAcum += delta * 2;
                vel.x = currentSpeedX * (movingAcum < 1 ? movingAcum : 1);
            }
            else if (IsKeyDown(KEY_LEFT)) {
                facingDirection = DIRECTION_LEFT;
                movingAcum += delta * 2;
                vel.x = -currentSpeedX * (movingAcum < 1 ? movingAcum : 1);
            }
            else {
                movingAcum = 0;
                if (vel.x >= -10 && vel.x <= 10) {
                    vel.x = 0;
                }
                else {
                    vel.x = vel.x * 0.9;
                }
            }

            if (state == SPRITE_STATE_ON_GROUND) {
                if (IsKeyDown(KEY_DOWN)) {
                    ducking = true;
                    vel.x = 0;
                }
                else {
                    ducking = false;
                }
            }

            if (IsKeyPressed(KEY_SPACE)) {
                if (state == SPRITE_STATE_ON_GROUND) {
                    vel.y = jumpSpeed;
                    state = SPRITE_STATE_JUMPING;
                    PlaySound(sounds["jump"]);
                }
            }

            if (IsKeyPressed(KEY_LEFT_CONTROL) && type == MARIO_TYPE_FLOWER) {

                if (facingDirection == DIRECTION_RIGHT) {
                    fireballs.push_back(Fireball(Vector2{ pos.x + dim.x / 2, pos.y + dim.y / 2 - 3 }, Vector2{ 16, 16 }, Vector2{ 400, 100 }, RED, DIRECTION_RIGHT, 2));
                }
                else {
                    fireballs.push_back(Fireball(Vector2{ pos.x, pos.y + dim.y / 2 - 3 }, Vector2{ 16, 16 }, Vector2{ -400, 100 }, RED, DIRECTION_LEFT, 2));
                }
                PlaySound(sounds["fireball"]);

            }

            std::vector<int> collectedIndexes;
            for (size_t i = 0; i < fireballs.size(); i++) {
                fireballs[i].update();
                if (fireballs[i].getState() == SPRITE_STATE_TO_BE_REMOVED) {
                    collectedIndexes.push_back(i);
                }
            }
            for (int i = collectedIndexes.size() - 1; i >= 0; i--) {
                fireballs.erase(fireballs.begin() + collectedIndexes[i]);
            }

            pos.x = pos.x + vel.x * delta;
            pos.y = pos.y + vel.y * delta;

            vel.y += GameWorld::gravity;

            if (static_cast<int>(lastPos.y) < static_cast<int>(pos.y)) {
                state = SPRITE_STATE_FALLING;
            }
            lastPos = pos;

        }

    }

}

void Mario::draw() {

    std::map<std::string, Texture2D>& textures = ResourceManager::getInstance().getTextures();
    std::string prefix;

    switch (type) {
    default:
    case MARIO_TYPE_SMALL:
        prefix = "small";
        break;
    case MARIO_TYPE_SUPER:
        prefix = "super";
        break;
    case MARIO_TYPE_FLOWER:
        prefix = "flower";
        break;
    }

    const char dir = facingDirection == DIRECTION_RIGHT ? 'R' : 'L';

    if (state == SPRITE_STATE_DYING) {
        DrawTexture(textures[std::string(TextFormat("smallMario%dDy", currentFrame))], pos.x, pos.y, WHITE);
    }
    else if (state == SPRITE_STATE_TRANSITIONING_SMALL_TO_SUPER ||
        state == SPRITE_STATE_TRANSITIONING_SUPER_TO_SMALL) {
        DrawTexture(textures[std::string(TextFormat("transitioningMarioSS%d%c", transitionCurrentFrame, dir))], pos.x, pos.y, WHITE);
    }
    else if (state == SPRITE_STATE_TRANSITIONING_SMALL_TO_FLOWER ||
        state == SPRITE_STATE_TRANSITIONING_FLOWER_TO_SMALL) {
        DrawTexture(textures[std::string(TextFormat("transitioningMarioSF%d%c", transitionCurrentFrame, dir))], pos.x, pos.y, WHITE);
    }
    else if (state == SPRITE_STATE_TRANSITIONING_SUPER_TO_FLOWER) {
        if (transitionCurrentFrame == 0) {
            DrawTexture(textures[std::string(TextFormat("superMario0%c", dir))], pos.x, pos.y, WHITE);
        }
        else {
            DrawTexture(textures[std::string(TextFormat("flowerMario0%c", dir))], pos.x, pos.y, WHITE);
        }
    }
    else {

        Color tint = WHITE;

        if (invincible) {
            tint = ColorFromHSV(360 * (invincibleAcum / invincibleTime * 20), 0.3, 1);
        }

		if (!invulnerableBlink) { // Only draw if not blinking

            if (state == SPRITE_STATE_ON_GROUND) {

                if (lookingUp) {
                    DrawTexture(textures[std::string(TextFormat("%sMario0Lu%c", prefix.c_str(), dir))], pos.x, pos.y, tint);
                }
                else if (ducking) {
                    DrawTexture(textures[std::string(TextFormat("%sMario0Du%c", prefix.c_str(), dir))], pos.x, pos.y, tint);
                }
                else if (drawRunningFrames) {
                    DrawTexture(textures[std::string(TextFormat("%sMario%dRu%c", prefix.c_str(), currentFrame, dir))], pos.x, pos.y, tint);
                }
                else { // idle
                    if (IsKeyPressed(KEY_LEFT_CONTROL) && type == MARIO_TYPE_FLOWER) {
                        DrawTexture(textures[std::string(TextFormat("%sMario%dTf%c", prefix.c_str(), currentFrame, dir))], pos.x, pos.y, tint);
                    }
                    else {
                        DrawTexture(textures[std::string(TextFormat("%sMario%d%c", prefix.c_str(), currentFrame, dir))], pos.x, pos.y, tint);
                    }
                }

            }
            else if (state == SPRITE_STATE_JUMPING) {
                if (drawRunningFrames) {
                    DrawTexture(textures[std::string(TextFormat("%sMario0JuRu%c", prefix.c_str(), dir))], pos.x, pos.y, tint);
                }
                else {
                    DrawTexture(textures[std::string(TextFormat("%sMario0Ju%c", prefix.c_str(), dir))], pos.x, pos.y, tint);
                }
            }
            else if (state == SPRITE_STATE_FALLING) {
                DrawTexture(textures[std::string(TextFormat("%sMario0Fa%c", prefix.c_str(), dir))], pos.x, pos.y, tint);
            }
            else if (state == SPRITE_STATE_VICTORY || state == SPRITE_STATE_WAITING_TO_NEXT_MAP) {
                DrawTexture(textures[std::string(TextFormat("%sMario0Vic", prefix.c_str()))], pos.x, pos.y, tint);
            }
            
        }

        for (auto& fireball : fireballs) {
            fireball.draw();
        }

    }

    if (playerDownMusicStreamPlaying) {
		playPlayerDownMusicStream();
    }
    else if (gameOverMusicStreamPlaying) {
		playGameOverMusicStream();
    }

}

CollisionType Mario::checkCollision(Sprite* sprite) {

    if (sprite->getState() != SPRITE_STATE_NO_COLLIDABLE) {

        Rectangle rect = sprite->getRect();

        for (auto& fireball : fireballs) {

            Fireball* f = &fireball;

            switch (f->checkCollision(sprite)) {
            case COLLISION_TYPE_NORTH:
                f->setVelY(-f->getVelY());
                break;
            case COLLISION_TYPE_SOUTH:
                f->setVelY(-300);
                break;
            case COLLISION_TYPE_EAST:
                f->setState(SPRITE_STATE_TO_BE_REMOVED);
                break;
            case COLLISION_TYPE_WEST:
                f->setState(SPRITE_STATE_TO_BE_REMOVED);
                break;
            default:
                break;
            }
        }

        if (cpN.checkCollision(rect)) {
            return COLLISION_TYPE_NORTH;
        }
        else if (cpS.checkCollision(rect)) {
            return COLLISION_TYPE_SOUTH;
        }
        else if (cpE.checkCollision(rect) || cpE1.checkCollision(rect)) {
            return COLLISION_TYPE_EAST;
        }
        else if (cpW.checkCollision(rect) || cpW1.checkCollision(rect)) {
            return COLLISION_TYPE_WEST;
        }

        // invisible blocks
    }
    else if (sprite->getAuxiliaryState() == SPRITE_STATE_INVISIBLE && state != SPRITE_STATE_FALLING) {
        Rectangle rect = sprite->getRect();
        if (cpN.checkCollision(rect)) {
            return COLLISION_TYPE_NORTH;
        }
    }

    return COLLISION_TYPE_NONE;

}

CollisionType Mario::checkCollisionBaddie(Sprite* sprite) {
    
    if (sprite->getState() != SPRITE_STATE_NO_COLLIDABLE) {
        Rectangle rect = sprite->getRect();

        for (auto& fireball : fireballs) {
            Fireball* f = &fireball;
            if (f->checkCollision(sprite) && sprite->getState() != SPRITE_STATE_DYING) {
                f->setState(SPRITE_STATE_TO_BE_REMOVED);
                return COLLISION_TYPE_FIREBALL;
            }
        }

        if (state == SPRITE_STATE_JUMPING || vel.y > 0) {
            if (cpN.checkCollision(rect)) {
				return COLLISION_TYPE_NORTH;
			}
            else if (cpS.checkCollision(rect)) {
				return COLLISION_TYPE_SOUTH;
			}
            else if (cpE.checkCollision(rect) || cpE1.checkCollision(rect)) {
				return COLLISION_TYPE_EAST;
			} 
            else if (cpW.checkCollision(rect) || cpW1.checkCollision(rect)) {
				return COLLISION_TYPE_WEST;
			}
        }
    }

	return COLLISION_TYPE_NONE;
}

void Mario::drawHud() const {
	std::map<std::string, Texture2D>& textures = ResourceManager::getInstance().getTextures();

    DrawTexture(textures["guiMario"], 34, 32, WHITE);
    DrawTexture(textures["guiX"], 54, 49, WHITE);
    drawWhiteSmallNumber(lives < 0 ? 0 : lives, 68, 49);

    for (int i = 0; i < yoshiCoins; i++) {
        DrawTexture(textures["guiCoin"], 34 + textures["guiMario"].width + 16 + i * textures["guiCoin"].width, 32, WHITE);
    }

	DrawTexture(textures["guiCoin"], GetScreenWidth() - 115, 32, WHITE);
    DrawTexture(textures["guiX"], GetScreenWidth() - 97, 34, WHITE);
    drawWhiteSmallNumber(coins, GetScreenWidth() - 34 - getSmallNumberWidth(coins), 34);
    drawWhiteSmallNumber(points, GetScreenWidth() - 34 - getSmallNumberWidth(points), 50);

	int t = getRemainingTime();
	t = t < 0 ? 0 : t;

    DrawTexture(textures["guiTime"], GetScreenWidth() - 34 - 176, 32, WHITE);
	drawYellowSmallNumber(t, GetScreenWidth() - 34 - 128 - getSmallNumberWidth(t), 50);

    if (reservedPowerUp == MARIO_TYPE_SUPER) {
        DrawTexture(textures["mushroom"], GetScreenWidth() / 2 - textures["mushroom"].width / 2, 32, WHITE);
    } else if (reservedPowerUp == MARIO_TYPE_FLOWER) {
        DrawTexture(textures["fireFlower0"], GetScreenWidth() / 2 - textures["fireFlower0"].width / 2, 32, WHITE);
    }
	DrawTexture(textures["guiNextItem"], GetScreenWidth() / 2 - textures["guiNextItem"].width / 2, 20, WHITE);
}

void Mario::updateCollisionProbes() {

    cpN.setX(pos.x + dim.x / 2 - cpN.getWidth() / 2);
    if (ducking) {
        cpN.setY(pos.y + dim.y - 32);
    }
    else {
        cpN.setY(pos.y);
    }

    cpS.setX(pos.x + dim.x / 2 - cpS.getWidth() / 2);
    cpS.setY(pos.y + dim.y - cpS.getHeight());

    cpE.setX(pos.x + dim.x - cpE.getWidth());
    cpE1.setX(pos.x + dim.x - cpE1.getWidth());
    cpW.setX(pos.x);
    cpW1.setX(pos.x);

    if (type == MARIO_TYPE_SMALL) {

        if (ducking) {
            cpE.setY(pos.y + 21 - cpE.getHeight() / 2);
            cpE1.setY(pos.y + 30 - cpE1.getHeight() / 2);
            cpW.setY(pos.y + 21 - cpW.getHeight() / 2);
            cpW1.setY(pos.y + 30 - cpW1.getHeight() / 2);
        }
        else {
            cpE.setY(pos.y + dim.y * 0.33 - cpE.getHeight() / 2);
            cpE1.setY(pos.y + dim.y * 0.66 - cpE1.getHeight() / 2);
            cpW.setY(pos.y + dim.y * 0.33 - cpW.getHeight() / 2);
            cpW1.setY(pos.y + dim.y * 0.66 - cpW1.getHeight() / 2);
        }

    }
    else {

        if (ducking) {
            cpE.setY(pos.y + 36 - cpE.getHeight() / 2);
            cpE1.setY(pos.y + 46 - cpE1.getHeight() / 2);
            cpW.setY(pos.y + 36 - cpW.getHeight() / 2);
            cpW1.setY(pos.y + 46 - cpW1.getHeight() / 2);
        }
        else {
            cpE.setY(pos.y + dim.y * 0.33 - cpE.getHeight() / 2);
            cpE1.setY(pos.y + dim.y * 0.66 - cpE1.getHeight() / 2);
            cpW.setY(pos.y + dim.y * 0.33 - cpW.getHeight() / 2);
            cpW1.setY(pos.y + dim.y * 0.66 - cpW1.getHeight() / 2);
        }

    }

}

float Mario::getSpeedX() const {
    return speedX;
}

float Mario::getMaxSpeedX() const {
    return maxSpeedX;
}

float Mario::getJumpSpeed() const {
    return jumpSpeed;
}

float Mario::getActivationWidth() const {
    return activationWidth;
}

void Mario::setImmortal(bool immortal) {
    this->immortal = immortal;
}

bool Mario::isImmortal() const {
    return immortal;
}

void Mario::setActivationWidth(float activationWidth) {
    this->activationWidth = activationWidth;
}

void Mario::setLives(int lives) {
    this->lives = lives;
}

void Mario::setCoins(int coins) {
    this->coins = coins;
}

void Mario::setYoshiCoins(int yoshiCoins) {
    this->yoshiCoins = yoshiCoins;
}

void Mario::setPoints(int points) {
    this->points = points;
}

int Mario::getRemainingTime() const {
    return static_cast<int>(maxTime - ellapsedTime);
}

void Mario::setMaxTime(float maxTime) {
    this->maxTime = maxTime;
}

void Mario::setLastStateBeforeTransition(SpriteState lastStateBeforeTransition) {
    this->lastStateBeforeTransition = lastStateBeforeTransition;
}

void Mario::setGameWorld(GameWorld* gw) {
    this->gw = gw;
}

void Mario::setMap(Map* map) {
    this->map = map;
}

GameWorld* Mario::getGameWorld() const {
    return gw;
}

Map* Mario::getMap() const {
    return map;
}

int Mario::getLives() const {
    return lives;
}

int Mario::getCoins() const {
    return coins;
}

int Mario::getYoshiCoins() const {
    return yoshiCoins;
}

int Mario::getPoints() const {
    return points;
}

void Mario::addLives(int lives) {
    this->lives += lives;
}

void Mario::removeLives(int lives) {
    this->lives -= lives;
}

void Mario::addCoins(int coins) {
    this->coins += coins;
}

void Mario::addYoshiCoins(int yoshiCoins) {
    this->yoshiCoins += yoshiCoins;
}

void Mario::removeCoins(int coins) {
    this->coins -= coins;
}

void Mario::addPoints(int points) {
    this->points += points;
}

void Mario::removePoints(int points) {
    this->points -= points;
}

void Mario::changeToSmall() {
    type = MARIO_TYPE_SMALL;
    pos.y = pos.y + 12;
    dim.y = 40;
    maxFrames = 2;
}

void Mario::changeToSuper() {
    type = MARIO_TYPE_SUPER;
    dim.y = 56;
    maxFrames = 3;
}

void Mario::changeToFlower() {
    type = MARIO_TYPE_FLOWER;
    dim.y = 56;
    maxFrames = 3;
}

void Mario::setReservedPowerUp(MarioType reservedPowerUp) {
    this->reservedPowerUp = reservedPowerUp;
}

MarioType Mario::getReservedPowerUp() const {
    return reservedPowerUp;
}

MarioType Mario::getType() const {
    return type;
}

void Mario::setInvulnerable(bool invulnerable) {
    this->invulnerable = invulnerable;
}

bool Mario::isInvulnerable() const {
    return invulnerable;
}

void Mario::setInvincible(bool invincible) {
    this->invincible = invincible;
}

bool Mario::isInvincible() const {
    return invincible;
}

bool Mario::isTransitioning() const {
    return state == SPRITE_STATE_TRANSITIONING_SMALL_TO_SUPER ||
        state == SPRITE_STATE_TRANSITIONING_SMALL_TO_FLOWER ||
        state == SPRITE_STATE_TRANSITIONING_SUPER_TO_FLOWER ||
        state == SPRITE_STATE_TRANSITIONING_SUPER_TO_SMALL ||
        state == SPRITE_STATE_TRANSITIONING_FLOWER_TO_SMALL;
}

void Mario::reset(bool removePowerUps) {

    if (removePowerUps) {
        changeToSmall();
        reservedPowerUp = MARIO_TYPE_SMALL;
    }
    vel.x = 0;
    vel.y = 0;
    dyingVelY = -600;
    state = SPRITE_STATE_ON_GROUND;
    facingDirection = DIRECTION_RIGHT;
    ducking = false;
    lookingUp = false;
    running = false;
    ellapsedTime = 0;
    invulnerable = false;
    invulnerableTimeAcum = 0;
    invulnerableBlink = false;
    yoshiCoins = 0;

}

void Mario::resetAll() {
    lives = 5;
    coins = 0;
    yoshiCoins = 0;
    points = 0;
    reset(true);
}

void Mario::playPlayerDownMusicStream() {

    std::map<std::string, Music> musics = ResourceManager::getInstance().getMusics();

    if (!playerDownMusicStreamPlaying) {
        playerDownMusicStreamPlaying = true;
    }
    else {
        if (!IsMusicStreamPlaying(musics["playerDown"])) {
            PlayMusicStream(musics["playerDown"]);
        }
        else {
            UpdateMusicStream(musics["playerDown"]);
            if (static_cast<int>(GetMusicTimeLength(musics["playerDown"])) == static_cast<int>(GetMusicTimePlayed(musics["playerDown"]))) {
                StopMusicStream(musics["playerDown"]);
                playerDownMusicStreamPlaying = false;
            }
        }
    }


}

void Mario::playGameOverMusicStream() {

    std::map<std::string, Music> musics = ResourceManager::getInstance().getMusics();

    if (!gameOverMusicStreamPlaying) {
        gameOverMusicStreamPlaying = true;
    }
    else {
        if (!IsMusicStreamPlaying(musics["gameOver"])) {
            PlayMusicStream(musics["gameOver"]);
        }
        else {
            UpdateMusicStream(musics["gameOver"]);
            if (static_cast<int>(GetMusicTimeLength(musics["gameOver"])) == static_cast<int>(GetMusicTimePlayed(musics["gameOver"]))) {
                StopMusicStream(musics["gameOver"]);
                gameOverMusicStreamPlaying = false;
            }
        }
    }

}

bool Mario::isPlayerDownMusicStreamPlaying() const {
    return playerDownMusicStreamPlaying;
}

bool Mario::isGameOverMusicStreamPlaying() const {
    return gameOverMusicStreamPlaying;
}

Vector2 Mario::getSouthCollisionProbePos() const {
    return Vector2{ cpS.getX(), cpS.getY() };
}