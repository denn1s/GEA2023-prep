#pragma once

#include <glm/glm.hpp>
#include <string>
#include "Game/Graphics/PixelShader.h"
#include "Game/Graphics/Texture.h"

struct NameComponent {
  std::string name;
};

struct TransformComponent {
  glm::vec2 position;
//  glm::vec2 scale;
//  float rotation;
};

struct SpeedComponent {
  int x;
  int y;
};

struct SimpleSpriteComponent {
  std::string name;
  PixelShader shader = { nullptr, "" };
};

struct SpriteComponent {
  std::string name;
  Uint32 animationFrames = 1;
  Uint32 animationDuration = 0;
  Uint32 xIndex = 0;
  Uint32 yIndex = 0;
  
  PixelShader shader = { nullptr, "" };
  Uint32 lastUpdate = 0;
};


// tilemap components
struct TilemapComponent {
    std::vector<Texture*> tiles;
    int mapWidth;
    int mapHeight;
    int tileSize;
};