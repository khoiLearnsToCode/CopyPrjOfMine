/**
 * @file Map.cpp
 * @author Prof. Dr. David Buzatto
 * @brief Map class implementation.
 *
 * @copyright Copyright (c) 2024
 */
//#include "Baddie.h"
//#include "BanzaiBill.h"
//#include "Block.h"
//#include "BlueKoopaTroopa.h"
//#include "BobOmb.h"
//#include "BulletBill.h"
//#include "BuzzyBeetle.h"
//#include "CloudBlock.h"
//#include "Coin.h"
//#include "CourseClearToken.h"
//#include "ExclamationBlock.h"
//#include "EyesClosedBlock.h"
//#include "EyesOpenedBlock.h"
//#include "FlyingGoomba.h"
#include "GameWorld.h"
//#include "GlassBlock.h"
//#include "Goomba.h"
//#include "GreenKoopaTroopa.h"
//#include "InvisibleBlock.h"
//#include "Item.h"
//#include "JumpingPiranhaPlant.h"
#include "Map.h"
//#include "MessageBlock.h"
//#include "MontyMole.h"
//#include "MummyBeetle.h"
//#include "Muncher.h"
//#include "PiranhaPlant.h"
//#include "QuestionBlock.h"
//#include "QuestionFireFlowerBlock.h"
//#include "QuestionMushroomBlock.h"
//#include "QuestionOneUpMushroomBlock.h"
//#include "QuestionStarBlock.h"
//#include "QuestionThreeUpMoonBlock.h"
#include "raylib.h"
//#include "RedKoopaTroopa.h"
#include "ResourceManager.h"
//#include "Rex.h"
#include "Sprite.h"
//#include "StoneBlock.h"
//#include "Swooper.h"
//#include "utils.h"
//#include "WoodBlock.h"
//#include "YellowKoopaTroopa.h"
//#include "YoshiCoin.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


Map::Map(Mario& mario, int id, bool loadTestMap, GameWorld* gw) :

    id(id),
    maxId(3),

    maxWidth(0),
    maxHeight(0),

    mario(mario),
    marioOffset(0),
    backgroundId(1),
    maxBackgroundId(10),
    backgroundColor(WHITE),
    backgroundTexture(Texture()),
    //drawBlackScreen(false),
    //drawBlackScreenFadeAcum(0),
    //drawBlackScreenFadeTime(1.5),

    //tileSetId(1),
    //maxTileSetId(4),

    musicId(0),
    maxMusicId(9),

    //parseBlocks(parseBlocks),
    //parseItems(parseItems),
    //parseBaddies(parseBaddies),

    loadTestMap(loadTestMap),
    parsed(false),

    camera(nullptr),
    gw(gw) {
}

Map::~Map() {

    for (const auto& tile : untouchableTiles) {
        delete tile;
    }

    for (const auto& tile : touchableTiles) {
        delete tile;
    }

    // for (const auto& backScenarioTile : backScenarioTiles) {
    //     delete backScenarioTile;
    // }

    // for (const auto& frontScenarioTile : frontScenarioTiles) {
    //     delete frontScenarioTile;
    // }

    //for (const auto& item : items) {
    //    delete item;
    //}

    //for (const auto& staticItem : staticItems) {
    //    delete staticItem;
    //}

    for (const auto& baddie : baddies) {
       delete baddie;
    }

    //for (const auto& block : blocks) {
    //    delete block;
    //}

}

void Map::loadFromJsonFile(bool shouldLoadTestMap) {
    //clear current map
    reset();

    loadTestMap = shouldLoadTestMap;

    std::string jsonFilePath;
    if (loadTestMap) {
        jsonFilePath = "../resource/maps/test.json";
    } 
    
    else {
        jsonFilePath = "../resource/maps/map" + std::to_string(id) + ".json";
    }

    std::ifstream fin(jsonFilePath);
    if (!fin){
        std::cout << "Cannot open " << jsonFilePath << std::endl;
        return;
    }

    nlohmann::json mapJson;
    try {
        fin >> mapJson;
    } catch (const std::exception& e) {
        std::cout << "Error parsing JSON file " << jsonFilePath << ": " << e.what() << std::endl;
        return;
    }

    // Check if required fields exist
    if (!mapJson.contains("width") || !mapJson.contains("height") || 
        !mapJson.contains("layers") || mapJson["layers"].empty() ||
        !mapJson["layers"][0].contains("data")) {
        std::cout << "JSON file " << jsonFilePath << " is missing required fields" << std::endl;
        return;
    }

    int width = mapJson["width"];
	int height = mapJson["height"];
    maxWidth = width * TILE_WIDTH;
    maxHeight = height * TILE_WIDTH;

    // Set background ID
    if (mapJson.contains("backgroundId")) {
        backgroundId = mapJson["backgroundId"];
        if (backgroundId < 0) {
            backgroundId = 0;
        } else if (backgroundId > maxBackgroundId) {
            backgroundId = maxBackgroundId;
        }
    } else {
        backgroundId = 1; // Default value
    }

    // Load tile IDs and dimensions
	int tilewidth = mapJson["tilewidth"];
    std::vector<int> tileIDsUntouchable = mapJson["layers"][0]["data"];

    // Load IDs as data
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int tileID = tileIDsUntouchable[y * width + x];
            if (tileID != 0){
                untouchableTiles.push_back(new Tile(Vector2{1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, Vector2{TILE_WIDTH * 1.0f, TILE_WIDTH * 1.0f},
                BLACK, "tile_" + std::to_string(tileID), true, TILE_TYPE_NON_SOLID));
            }
        }
    } 

    tileIDsUntouchable.clear();

    // Load touchable tiles
    std::vector<int> tileIDsTouchable = mapJson["layers"][2]["data"];

    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int tileID = tileIDsTouchable[y * width + x];
            if (tileID != 0){
                touchableTiles.push_back(new Tile(Vector2{1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, Vector2{TILE_WIDTH * 1.0f, TILE_WIDTH * 1.0f},
                BLACK, "tile_" + std::to_string(tileID), true));
            }
        }
    }

    tileIDsTouchable.clear();

    std::vector<int> backBaddieIDs = mapJson["layers"][1]["data"];

    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int baddieID = backBaddieIDs[y * width + x];
            if (baddieID != 146 && baddieID != 136) continue;

            Baddie* newBaddie;

            if (baddieID == 146) { // Piranha Plant
                newBaddie = new PiranhaPlant({1.0f * x * TILE_WIDTH + 16, 1.0f * y * TILE_WIDTH + 36}, {32,66}, {0,0}, RED);
            } 

            else if (baddieID == 136) { // Jumping Piranha Plant
                newBaddie = new JumpingPiranhaPlant({1.0f * x * TILE_WIDTH + 16, 1.0f * y * TILE_WIDTH + 34}, {32,42}, {0,0}, RED);
            } 

            backBaddies.push_back(newBaddie);
            baddies.push_back(newBaddie);
        }
    }

    backBaddieIDs.clear();

    std::vector<int> frontBaddieIDs = mapJson["layers"][5]["data"];

    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int baddieID = frontBaddieIDs[y * width + x];
            if (baddieID == 0 || baddieID < 121 || baddieID > 158) continue;

            Baddie* newBaddie;
            if (baddieID == 121) {
                newBaddie = new BlueKoopaTroopa({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,54}, {-100,0}, BLUE);
            } 

            else if (baddieID == 123) {
                newBaddie = new BobOmb({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {24,30}, {-100,0}, BLACK);
            } 

            else if (baddieID == 125) { //bullet bill
                newBaddie = new BulletBill({1.0f * x * TILE_WIDTH, 1.0f * (y+2) * TILE_WIDTH }, {32,28}, {-200,0}, BLACK);
            } 

            else if (baddieID == 126) { // buzzy beetle
                newBaddie = new BuzzyBeetle({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,32}, {-80,0}, BLUE);
            }

            else if (baddieID == 128) { //flying gomba
                newBaddie = new FlyingGoomba({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {66,50}, {-100,0}, MAROON);
            } 

            else if (baddieID == 132) { //goomba
                newBaddie = new Goomba({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,30}, {-100,0}, MAROON);
            } 

            else if (baddieID == 134) { //green koopa troopa
                newBaddie = new GreenKoopaTroopa({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,54}, {-100,0}, GREEN);
            } 

            else if (baddieID == 140) { //monty mole
                newBaddie = new MontyMole({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,30}, {-200,0}, BROWN);
            } 

            else if (baddieID == 142) { // mummy beetle
                newBaddie = new MummyBeetle({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,32}, {-80,0}, GRAY);
            } 

            else if (baddieID == 144) { //muncher
                newBaddie = new Muncher({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,30}, {-80,0}, BROWN);
            }

            else if (baddieID == 148) { // red koopa troopa
                newBaddie = new RedKoopaTroopa({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,54}, {-100,0}, RED);
            } 

            else if (baddieID == 152) { //rex
                newBaddie = new Rex({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {40,64}, {-100,0}, VIOLET);
            }

            else if (baddieID == 154) { //swooper
                newBaddie = new Swooper({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,34}, {-100,0}, BLACK);
            } 

            else if (baddieID == 157) { // yellow koopa troopa
                newBaddie = new YellowKoopaTroopa({1.0f * x * TILE_WIDTH, 1.0f * y * TILE_WIDTH}, {32,54}, {-100,0}, YELLOW);
            } 
            
            else {
                std::cerr << "Unsupported baddie ID: " << baddieID << " at position (" << x << ", " << y << ")" << std::endl;
                continue; // Skip unsupported baddies
            }
            frontBaddies.push_back(newBaddie);
            baddies.push_back(newBaddie);
        }
    }

    fin.close();
}

void Map::draw() {

    DrawRectangle(0, 0, maxWidth, maxHeight, backgroundColor);

    std::string backgroundTextureKey = "background" + std::to_string(backgroundId);
    if (ResourceManager::getInstance().getTextures().find(backgroundTextureKey) != ResourceManager::getInstance().getTextures().end()) {
        backgroundTexture = ResourceManager::getInstance().getTexture(backgroundTextureKey);
    } else {
        backgroundTexture = Texture2D(); // Fallback to an empty texture if not found
        std::cout << "Background texture not found: " << backgroundTextureKey << std::endl;
    }

    if (backgroundId > 0) {
        const int repeats = maxWidth / backgroundTexture.width + 2;
        for (int i = 0; i <= repeats; i++) {
            DrawTexture(
                backgroundTexture,
                -backgroundTexture.width + i * backgroundTexture.width - marioOffset * 0.06,
                maxHeight - backgroundTexture.height,
                WHITE);
        }
    }

    //for (const auto& backScenarioTile : backScenarioTiles) {
    //    backScenarioTile->draw();
    //}

    //for (const auto& baddie : backBaddies) {
    //    baddie->draw();
    //}

    for (const auto& tile : untouchableTiles) {
        tile->draw();
    }

    for (const auto& tile : touchableTiles) {
        tile->draw();
    }

    //for (const auto& block : blocks) {
    //    block->draw();
    //}

    //for (const auto& item : items) {
    //    item->draw();
    //}

    //for (const auto& staticItem : staticItems) {
    //    staticItem->draw();
    //}

    //for (const auto& baddie : frontBaddies) {
    //    baddie->draw();
    //}

    mario.draw();

    // for (const auto& frontScenarioTile : frontScenarioTiles) {
    //     frontScenarioTile->draw();
    // }

    //if (drawBlackScreen) {
    //    if (drawBlackScreenFadeAcum < drawBlackScreenFadeTime) {
    //        drawBlackScreenFadeAcum += GetFrameTime();
    //    }
    //    DrawRectangle(0, 0, maxWidth, maxHeight, Fade(BLACK, 0.5 * drawBlackScreenFadeAcum / drawBlackScreenFadeTime));
    //}

    //if (drawMessage) {

    //    std::vector<std::string> messages = split(message, "\\n");
    //    const Vector2 center = GetScreenToWorld2D(Vector2(GetScreenWidth() / 2, GetScreenHeight() / 2), *camera);
    //    int currentLine = 0;
    //    const int margin = 10;
    //    const int vSpacing = 5;

    //    int maxWidth = 0;
    //    int maxHeight = messages.size() * getDrawMessageStringHeight() + (messages.size() - 1) * vSpacing;

    //    for (const auto& m : messages) {
    //        const int w = getDrawMessageStringWidth(m);
    //        if (maxWidth < w) {
    //            maxWidth = w;
    //        }
    //    }

    //    const int xStart = center.x - maxWidth / 2 + margin;
    //    const int yStart = center.y - maxHeight / 2 + margin - 50;

    //    DrawRectangle(xStart - margin, yStart - margin, maxWidth + margin * 2, maxHeight + margin * 2, BLACK);

    //    for (const auto& m : messages) {
    //        drawMessageString(m,
    //            xStart,
    //            yStart + currentLine * getDrawMessageStringHeight() + (currentLine < (int)messages.size() ? currentLine * vSpacing : 0));
    //        currentLine++;
    //    }

    //}

    //if (GameWorld::debug) {
    //    int columns = maxWidth / TILE_WIDTH;
    //    int lines = maxHeight / TILE_WIDTH;
    //    for (int i = 0; i < lines; i++) {
    //        DrawLine(0, i * TILE_WIDTH, maxWidth, i * TILE_WIDTH, BLACK);
    //    }
    //    for (int i = 0; i < columns; i++) {
    //        DrawLine(i * TILE_WIDTH, 0, i * TILE_WIDTH, maxHeight, BLACK);
    //    }
    //}

}

std::vector<Tile*>& Map::getTiles() {
    return touchableTiles;
}

// std::vector<Block*>& Map::getBlocks() {
//     return blocks;
// }

//std::vector<Item*>& Map::getItems() {
//    return items;
//}

//std::vector<Item*>& Map::getStaticItems() {
//    return staticItems;
//}

std::vector<Baddie*>& Map::getBaddies() {
   return baddies;
}

//void Map::playMusic() const {
//
//    if (musicId != 0) {
//
//        std::map<std::string, Music> musics = ResourceManager::getMusics();
//        const std::string key(TextFormat("music%d", musicId));
//
//        if (mario.isInvincible()) {
//            if (IsMusicStreamPlaying(musics[key])) {
//                StopMusicStream(musics[key]);
//            }
//            if (!IsMusicStreamPlaying(musics["invincible"])) {
//                PlayMusicStream(musics["invincible"]);
//                SeekMusicStream(musics["invincible"], 1);
//            }
//            else {
//                UpdateMusicStream(musics["invincible"]);
//            }
//        }
//        else {
//            if (!IsMusicStreamPlaying(musics[key])) {
//                StopMusicStream(musics["invincible"]);
//                PlayMusicStream(musics[key]);
//            }
//            else {
//                UpdateMusicStream(musics[key]);
//            }
//        }
//
//    }
//}

//void Map::parseMap() {
//
//    if (!parsed) {
//
//        char* mapData;
//        std::vector<std::string> blockMessages;
//        int messagePosition = 0;
//
//        if (loadTestMap) {
//            mapData = LoadFileText(TextFormat("resources/maps/mapTests.txt"));
//        }
//        else {
//            mapData = LoadFileText(TextFormat("resources/maps/map%d.txt", id));
//        }
//
//        std::map<std::string, Texture2D>& textures = ResourceManager::getTextures();
//
//        int currentColumn = 0;
//        int currentLine = 0;
//        bool ignoreLine = false;
//
//        while (*mapData != '\0') {
//
//            const float x = currentColumn * TILE_WIDTH;
//            const float y = currentLine * TILE_WIDTH;
//
//            if (*mapData == '#') {
//                ignoreLine = true;
//            }
//
//            if (currentLine == 0 && currentColumn == 0) {
//
//                if (*mapData == 'c') {            // parse color
//
//                    ignoreLine = true;
//                    mapData += 3;
//                    std::string hexColor;
//
//                    while (*mapData != ' ') {
//                        hexColor += std::string(1, *mapData);
//                        mapData++;
//                    }
//
//                    backgroundColor = GetColor(std::stoul(hexColor, nullptr, 16));
//                    currentColumn = 1;
//
//                }
//                else if (*mapData == 'b') {     // parse background id
//
//                    ignoreLine = true;
//                    mapData += 3;
//                    std::string number;
//
//                    while (*mapData != ' ') {
//                        number += std::string(1, *mapData);
//                        mapData++;
//                    }
//                    backgroundId = std::stoi(number);
//                    if (backgroundId < 0) {
//                        backgroundId = 0;
//                    }
//                    else if (backgroundId > maxBackgroundId) {
//                        backgroundId = maxBackgroundId;
//                    }
//
//                    if (backgroundId > 0) {
//                        backgroundTexture = textures[TextFormat("background%d", backgroundId)];
//                    }
//                    currentColumn = 1;
//
//                }
//                else if (*mapData == 't') {     // parse tile set id
//
//                    ignoreLine = true;
//                    mapData += 3;
//                    std::string number;
//
//                    while (*mapData != ' ') {
//                        number += std::string(1, *mapData);
//                        mapData++;
//                    }
//                    tileSetId = std::stoi(number);
//                    if (tileSetId <= 0) {
//                        tileSetId = 1;
//                    }
//                    else if (tileSetId > maxTileSetId) {
//                        tileSetId = maxTileSetId;
//                    }
//
//                    currentColumn = 1;
//
//                }
//                else if (*mapData == 'm') {     // parse music id
//
//                    ignoreLine = true;
//                    mapData += 3;
//                    std::string number;
//
//                    while (*mapData != ' ') {
//                        number += std::string(1, *mapData);
//                        mapData++;
//                    }
//                    musicId = std::stoi(number);
//                    if (musicId < 0) {
//                        musicId = 0;
//                    }
//                    else if (musicId > maxMusicId) {
//                        musicId = maxMusicId;
//                    }
//
//                    currentColumn = 1;
//
//                }
//                else if (*mapData == 'f') {     // parse time to finish
//
//                    ignoreLine = true;
//                    mapData += 3;
//                    std::string number;
//
//                    while (*mapData != ' ') {
//                        number += std::string(1, *mapData);
//                        mapData++;
//                    }
//                    mario.setMaxTime(std::stoi(number));
//
//                    currentColumn = 1;
//
//                }
//                else if (*mapData == 'h') {     // parse map messages
//
//                    ignoreLine = true;
//                    mapData += 3;
//                    std::string currentMessage;
//
//                    while (*mapData != '\n') {
//                        currentMessage += std::string(1, *mapData);
//                        mapData++;
//                    }
//
//                    blockMessages.push_back(currentMessage);
//                    currentColumn = 1;
//
//                }
//
//            }
//
//            if (!ignoreLine) {
//
//                std::string blockMessage;
//                MessageBlock* newMessageBlock;
//                Baddie* newBaddie;
//
//                // processing boundary tiles when used as first column
//                // for camera adjustment
//                if (*mapData != '/') {
//                    if (maxWidth < x) {
//                        maxWidth = x;
//                    }
//                    if (maxHeight < y) {
//                        maxHeight = y;
//                    }
//                }
//
//                switch (*mapData) {
//
//                    // test tiles
//                case 'a':
//                    tiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), GREEN, "", true, TILE_TYPE_SOLID_FROM_ABOVE));
//                    break;
//                case 'b':
//                    tiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH * 4, TILE_WIDTH * 4), BLUE, "", true, TILE_TYPE_SLOPE_UP));
//                    break;
//                case 'd':
//                    tiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH * 4, TILE_WIDTH * 4), ORANGE, "", true, TILE_TYPE_SLOPE_DOWN));
//                    break;
//                case 'e':
//                    tiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), PURPLE, "", true, TILE_TYPE_NON_SOLID));
//                    break;
//                    /*case 'c':
//                        tiles.push_back( new Tile( Vector2( x, y ), Vector2( TILE_WIDTH, TILE_WIDTH ), RED, "", true ) );
//                        break;
//                    case 'd':
//                        tiles.push_back( new Tile( Vector2( x, y ), Vector2( TILE_WIDTH, TILE_WIDTH ), ORANGE, "", true ) );
//                        break;*/
//
//                        // blocks
//                case 'i':
//                    if (parseBlocks) blocks.push_back(new EyesClosedBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'y':
//                    if (parseBlocks) blocks.push_back(new EyesOpenedBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 's':
//                    if (parseBlocks) blocks.push_back(new StoneBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'w':
//                    if (parseBlocks) blocks.push_back(new WoodBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'g':
//                    if (parseBlocks) blocks.push_back(new GlassBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'c':
//                    if (parseBlocks) blocks.push_back(new CloudBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'v':
//                    if (parseBlocks) if (parseBlocks) blocks.push_back(new InvisibleBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'h':
//
//                    if (messagePosition >= 0 && messagePosition < (int)blockMessages.size()) {
//                        blockMessage = blockMessages[messagePosition];
//                    }
//
//                    newMessageBlock = new MessageBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR, blockMessage);
//
//                    if (parseBlocks) {
//                        blocks.push_back(newMessageBlock);
//                        messageBlocks.push_back(newMessageBlock);
//                    }
//
//                    messagePosition++;
//
//                    break;
//
//                case '!':
//                    if (parseBlocks) blocks.push_back(new ExclamationBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case '?':
//                    if (parseBlocks) blocks.push_back(new QuestionBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'm':
//                    if (parseBlocks) blocks.push_back(new QuestionMushroomBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'f':
//                    if (parseBlocks) blocks.push_back(new QuestionFireFlowerBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case 'u':
//                    if (parseBlocks) blocks.push_back(new QuestionOneUpMushroomBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case '+':
//                    if (parseBlocks) blocks.push_back(new QuestionThreeUpMoonBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//                case '*':
//                    if (parseBlocks) blocks.push_back(new QuestionStarBlock(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR));
//                    break;
//
//                    // bondarie tiles
//                case '/':
//                    tiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), WHITE, "", false));
//                    break;
//                case '|':
//                    tiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), BLACK, "", false, TILE_TYPE_SOLID_ONLY_FOR_BADDIES));
//                    break;
//
//                    // scenario tiles
//                case '{': backScenarioTiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR, "tileCourseClearPoleBackTop", true));
//                    break;
//                case '[': backScenarioTiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR, "tileCourseClearPoleBackBody", true));
//                    break;
//                case '}': frontScenarioTiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR, "tileCourseClearPoleFrontTop", true));
//                    break;
//                case ']': frontScenarioTiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR, "tileCourseClearPoleFrontBody", true));
//                    break;
//
//                    // tiles from A to Z (depends on tile set parameter)
//                default:
//                    if (*mapData >= 'A' && *mapData <= 'Z') {
//                        std::stringstream ss;
//                        ss << *mapData << tileSetId;
//                        tiles.push_back(new Tile(Vector2(x, y), Vector2(TILE_WIDTH, TILE_WIDTH), DEBUGGABLE_TILE_COLOR, ss.str(), true));
//                    }
//                    break;
//
//                    // items
//                case 'o':
//                    if (parseItems) staticItems.push_back(new Coin(Vector2(x + 4, y), Vector2(24, 32), YELLOW));
//                    break;
//                case ':':
//                    if (parseItems) staticItems.push_back(new YoshiCoin(Vector2(x, y + 5), Vector2(32, 50), YELLOW));
//                    break;
//                case '=':
//                    if (parseItems) staticItems.push_back(new CourseClearToken(Vector2(x - TILE_WIDTH, y), Vector2(64, 32), LIGHTGRAY));
//                    break;
//                    /*case 'm':
//                        items.push_back( new Mushroom( Vector2( x, y ), Vector2( 32, 32 ), Vector2( 200, 0 ), RED ) );
//                        break;
//                    case 'f':
//                        items.push_back( new FireFlower( Vector2( x, y ), Vector2( 32, 32 ), ORANGE ) );
//                        break;
//                    case 'u':
//                        items.push_back( new OneUpMushroom( Vector2( x, y ), Vector2( 32, 32 ), Vector2( 250, 0 ), GREEN ) );
//                        break;
//                    case '+':
//                        items.push_back( new ThreeUpMoon( Vector2( x, y ), Vector2( 30, 32 ), Vector2( 300, 0 ), YELLOW ) );
//                        break;
//                    case '*':
//                        if ( parseItems ) staticItems.push_back( new Star( Vector2( x, y ), Vector2( 30, 32 ), Vector2( 300, 0 ), YELLOW ) );
//                        break;*/
//
//                        // baddies
//                case '1':
//                    if (parseBaddies) {
//                        newBaddie = new Goomba(Vector2(x, y), Vector2(32, 30), Vector2(-100, 0), MAROON);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '2':
//                    if (parseBaddies) {
//                        newBaddie = new FlyingGoomba(Vector2(x, y), Vector2(66, 50), Vector2(-100, 0), MAROON);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '3':
//                    if (parseBaddies) {
//                        newBaddie = new GreenKoopaTroopa(Vector2(x, y), Vector2(32, 54), Vector2(-100, 0), GREEN);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '4':
//                    if (parseBaddies) {
//                        newBaddie = new RedKoopaTroopa(Vector2(x, y), Vector2(32, 54), Vector2(-100, 0), RED);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '5':
//                    if (parseBaddies) {
//                        newBaddie = new BlueKoopaTroopa(Vector2(x, y), Vector2(32, 54), Vector2(-100, 0), BLUE);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '6':
//                    if (parseBaddies) {
//                        newBaddie = new YellowKoopaTroopa(Vector2(x, y), Vector2(32, 54), Vector2(-100, 0), YELLOW);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '7':
//                    if (parseBaddies) {
//                        newBaddie = new BobOmb(Vector2(x, y), Vector2(24, 30), Vector2(-100, 0), BLACK);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '8':
//                    if (parseBaddies) {
//                        newBaddie = new BulletBill(Vector2(x, y + 2), Vector2(32, 28), Vector2(-200, 0), BLACK);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '9':
//                    if (parseBaddies) {
//                        newBaddie = new Swooper(Vector2(x, y), Vector2(32, 34), Vector2(-100, 0), GREEN);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '@':
//                    if (parseBaddies) {
//                        newBaddie = new BuzzyBeetle(Vector2(x, y), Vector2(32, 32), Vector2(-80, 0), BLUE);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '$':
//                    if (parseBaddies) {
//                        newBaddie = new MummyBeetle(Vector2(x, y), Vector2(32, 32), Vector2(-80, 0), GRAY);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '%':
//                    if (parseBaddies) {
//                        newBaddie = new Rex(Vector2(x, y), Vector2(40, 64), Vector2(-100, 0), VIOLET);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '&':
//                    if (parseBaddies) {
//                        newBaddie = new Muncher(Vector2(x, y), Vector2(32, 30), BROWN);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '~':
//                    if (parseBaddies) {
//                        newBaddie = new PiranhaPlant(Vector2(x + 16, y + 36), Vector2(32, 66), RED);
//                        baddies.push_back(newBaddie);
//                        backBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '^':
//                    if (parseBaddies) {
//                        newBaddie = new JumpingPiranhaPlant(Vector2(x + 16, y + 34), Vector2(32, 42), RED);
//                        baddies.push_back(newBaddie);
//                        backBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '<':
//                    if (parseBaddies) {
//                        newBaddie = new BanzaiBill(Vector2(x, y), Vector2(128, 128), Vector2(-200, 0), BLACK);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//                case '.':
//                    if (parseBaddies) {
//                        newBaddie = new MontyMole(Vector2(x, y), Vector2(32, 30), Vector2(-200, 0), BROWN);
//                        baddies.push_back(newBaddie);
//                        frontBaddies.push_back(newBaddie);
//                    }
//                    break;
//
//                    // no tile
//                case ' ':
//                    break;
//
//                    // mario/player
//                case 'p':
//                    mario.setPos(Vector2(x, y));
//                    break;
//
//                    // parsing control
//                case '\n':
//                    currentLine++;
//                    currentColumn = -1;
//                    ignoreLine = false;
//                    break;
//
//                }
//
//            }
//
//            if (ignoreLine && *mapData == '\n') {
//                ignoreLine = false;
//                currentColumn = -1;
//            }
//
//            currentColumn++;
//            mapData++;
//
//        }
//
//        maxWidth -= TILE_WIDTH;
//        maxHeight += TILE_WIDTH;
//        parsed = true;
//
//    }
//
//}

float Map::getMaxWidth() const {
    return maxWidth;
}

float Map::getMaxHeight() const {
    return maxHeight;
}

void Map::setMarioOffset(float marioOffset) {
   this->marioOffset = marioOffset;
}

//void Map::setDrawBlackScreen(bool drawBlackScreen) {
//    this->drawBlackScreen = drawBlackScreen;
//}

//void Map::setDrawMessage(bool drawMessage) {
//    this->drawMessage = drawMessage;
//    for (const auto& mb : messageBlocks) {
//        mb->resetHit();
//    }
//}

//void Map::setMessage(std::string message) {
//    this->message = std::move(message);
//}

void Map::setCamera(Camera2D* camera) {
    this->camera = camera;
}

void Map::setGameWorld(GameWorld* gw) {
    this->gw = gw;
}

int Map::getId() const {
    return id;
}

void Map::reset() {

    maxWidth = 0;
    maxHeight = 0;
    marioOffset = 0;
    //drawBlackScreen = false;
    //drawBlackScreenFadeAcum = 0;

    for (const auto& tile : untouchableTiles) {
        delete tile;
    }
    untouchableTiles.clear();

    for (const auto& tile : touchableTiles) {
        delete tile;
    }
    touchableTiles.clear();

    // for (const auto& backScenarioTile : backScenarioTiles) {
    //     delete backScenarioTile;
    // }
    // backScenarioTiles.clear();

    // for (const auto& frontScenarioTile : frontScenarioTiles) {
    //     delete frontScenarioTile;
    // }
    // frontScenarioTiles.clear();

    //for (const auto& block : blocks) {
    //    delete block;
    //}
    //blocks.clear();
    //messageBlocks.clear();

    //for (const auto& item : items) {
    //    delete item;
    //}
    //items.clear();

    //for (const auto& staticItem : staticItems) {
    //    delete staticItem;
    //}
    //staticItems.clear();

    for (const auto& baddie : baddies) {
       delete baddie;
    }
    baddies.clear();
    frontBaddies.clear();
    backBaddies.clear();

//    StopMusicStream(ResourceManager::getMusics()[std::string(TextFormat("music%d", musicId))]);
    parsed = false;
    //parseMap();

}

bool Map::hasNext() {

    id++;

    if (id > maxId) {
        return false;
    }
    else {
        reset();
    }

    return true;

}

void Map::first() {
    id = 1;
}

//void Map::pauseGameToShowMessage() const {
//    gw->pauseGame(false, false, false, true);
//}

//void Map::eraseBaddieFromDrawingVectors(Baddie* baddie) {
//
//    for (size_t i = 0; i < frontBaddies.size(); i++) {
//        if (frontBaddies[i] == baddie) {
//            frontBaddies.erase(frontBaddies.begin() + i);
//            return;
//        }
//    }
//
//    for (size_t i = 0; i < backBaddies.size(); i++) {
//        if (backBaddies[i] == baddie) {
//            backBaddies.erase(backBaddies.begin() + i);
//            return;
//        }
//    }
//
//}
