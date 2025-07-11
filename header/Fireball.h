#pragma once

#include "Direction.h"
#include "raylib.h"
#include "Sprite.h"

class Fireball : public Sprite {

    float timeSpan;
    float timeSpanAcum;

public:

    Fireball(Vector2 pos, Vector2 dim, Vector2 vel, Color color, Direction facingDirection, float timeSpan);
    ~Fireball() override;

    void update() override;
    void draw() override;
    void updateCollisionProbes() override;

};