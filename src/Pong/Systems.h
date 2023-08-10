#pragma once
#include <print.h>
#include "ECS/Entity.h"
#include "Components.h"

#include "ECS/SystemTypes/SystemTypes.h"

#include "Game/Graphics/TextureManager.h"

#include <FastNoiseLite.h> // include the FastNoise library
#include <random>

class HelloWorldSystem : public SetupSystem {
  public:
    HelloWorldSystem() {
      print("Hello World Constructor");
    }

    ~HelloWorldSystem() {
      print("Hello World Destructor");
    }

    void run() {
      print("Hello World run!");
    }
};

class RectRenderSystem : public RenderSystem {
  void run(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 1);

    const auto view = scene->r.view<TransformComponent, SizeComponent>();

    for (const auto entity : view) {
      const auto t = view.get<TransformComponent>(entity);
      const auto s = view.get<SizeComponent>(entity);


      int x = t.position.x;
      int y = t.position.y;
      int w = s.w;
      int h = s.h;

      SDL_Rect rect = { x, y, w, h};
      SDL_RenderFillRect(renderer, &rect);
    }
  }
};

class MovementUpdateSystem : public UpdateSystem {
  public:
    MovementUpdateSystem(int screen_width, int screen_height) 
    : screen_width(screen_width), screen_height(screen_height) {}

    void run(float dT) {
      const auto view = scene->r.view<TransformComponent, SpeedComponent, SizeComponent>();

      for (const auto entity : view) {
        auto& t = view.get<TransformComponent>(entity);
        auto& s = view.get<SpeedComponent>(entity);
        auto& sz = view.get<SizeComponent>(entity);


        if (s.x == 0 && s.y == 0) {
          continue;
        }

        const int nx = t.position.x + s.x * dT;
        const int ny = t.position.y + s.y * dT;

        if (nx <= 0) {
          s.x *= -1.2;
        }
        if (nx + sz.w >= screen_width) {
          s.x *= -1.2;
        }
        if (ny <= 0) {
          s.y *= -1.2;
        }
        if (ny + sz.h > screen_height) {
          exit(1);
        }

        t.position.x = nx;
        t.position.y = ny;
      }
    }
  private:
    int screen_width;
    int screen_height;
};


class PlayerInputSystem : public EventSystem {
  void run(SDL_Event event) {
    scene->r.view<SpeedComponent, PlayerComponent>()
      .each(
        [&](const auto entity, auto& s, auto& p) {
          if (event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.sym) {
              case SDLK_RIGHT:
                s.x = p.moveSpeed;
                break;
              case SDLK_LEFT:
                s.x = -p.moveSpeed;
                break;
            }
          }
          if (event.type == SDL_KEYUP) {
            s.x = 0;
          }
        }
      );
    }
};

class CollisionDetectionUpdateSystem : public UpdateSystem {
  void run(float dT) {
    scene->r.view<TransformComponent, SizeComponent, ColliderComponent>()
      .each(
        [&](const auto entity,
            auto& transformComponent,
            auto& sizeComponent,
            auto& colliderComponent
          ) {
            // cada entidad que tiene collider
            // AABB
            scene->r.view<TransformComponent, SpeedComponent, SizeComponent>()
            .each(
              [&](const auto entity2,
                  auto& transformComponent2,
                  auto& speedComponent2,
                  auto& sizeComponent2
                ) {
                  if (entity == entity2) {
                    // skip self collision
                    return;
                  }

                  SDL_Rect boxCol1 = {
                    static_cast<int>(transformComponent.position.x),
                    static_cast<int>(transformComponent.position.y),
                    sizeComponent.w,
                    sizeComponent.h
                  };

                  SDL_Rect boxCol2 = {
                    static_cast<int>(transformComponent2.position.x),
                    static_cast<int>(transformComponent2.position.y),
                    sizeComponent2.w,
                    sizeComponent2.h
                  };

                  if (SDL_HasIntersection(&boxCol1, &boxCol2)) {
                    colliderComponent.triggered = true;
                    colliderComponent.transferSpeed = speedComponent2.x;
                  }

              }
            );
        }
      );
    }
};

class BounceUpdateSystem : public UpdateSystem {
  void run(float dT) {
    scene->r.view<TransformComponent, SpeedComponent, ColliderComponent>()
      .each(
        [&](const auto entity,
            auto& transformComponent,
            auto& speedComponent,
            auto& colliderComponent
          ) {
            if (colliderComponent.triggered) {
              speedComponent.y *= -1.5;
              speedComponent.x += colliderComponent.transferSpeed;

              colliderComponent.triggered = false;
            }
          }
      );
    }
};

class SimpleSpriteSetupSystem : public SetupSystem {
  public:
    SimpleSpriteSetupSystem(SDL_Renderer* renderer, SDL_Window* window)
      : renderer(renderer), window(window) { }

    ~SimpleSpriteSetupSystem() {
      auto view = scene->r.view<SimpleSpriteComponent>();

      for(auto entity : view) {
        const auto spriteComponent = view.get<SimpleSpriteComponent>(entity);
  
        TextureManager::UnloadTexture(spriteComponent.name);
      }
    }

    void run() {
      auto view = scene->r.view<SimpleSpriteComponent>();

      for(auto entity : view) {
        const auto spriteComponent = view.get<SimpleSpriteComponent>(entity);
  
        TextureManager::LoadTexture(spriteComponent.name, renderer, window, spriteComponent.shader);
      }
    }

  private:
    SDL_Renderer* renderer;
    SDL_Window* window;
};


class SimpleSpriteRenderSystem : public RenderSystem {
  public:
    void run(SDL_Renderer* renderer) {
      auto view = scene->r.view<TransformComponent, SimpleSpriteComponent>();

      for(auto entity : view) {
        const auto spriteComponent = view.get<SimpleSpriteComponent>(entity);
        const auto transformComponent = view.get<TransformComponent>(entity);
  
        Texture* texture = TextureManager::GetTexture(spriteComponent.name, spriteComponent.shader.name);
  
        texture->render(
          transformComponent.position.x,
          transformComponent.position.y,
          100,
          100
        );
      }
    }
};

/*

struct SpriteComponent {
  std::string name;
  Uint32 animationFrames = 1;
  Uint32 animationDuration = 0;
  Uint32 xIndex = 0;
  Uint32 yIndex = 0;
  
  PixelShader shader = { nullptr, "" };
  Uint32 lastUpdate = 0;
};


*/

class SpriteSetupSystem : public SetupSystem {
  public:
    SpriteSetupSystem(SDL_Renderer* renderer, SDL_Window* window)
      : renderer(renderer), window(window) { }

    ~SpriteSetupSystem() {
      auto view = scene->r.view<SpriteComponent>();

      for(auto entity : view) {
        const auto spriteComponent = view.get<SpriteComponent>(entity);
  
        TextureManager::UnloadTexture(spriteComponent.name);
      }
    }

    void run() {
      auto view = scene->r.view<SpriteComponent>();

      for(auto entity : view) {
        const auto spriteComponent = view.get<SpriteComponent>(entity);
  
        TextureManager::LoadTexture(spriteComponent.name, renderer, window, spriteComponent.shader);
      }
    }

  private:
    SDL_Renderer* renderer;
    SDL_Window* window;
};


class SpriteRenderSystem : public RenderSystem {
  public:
    void run(SDL_Renderer* renderer) {
      auto view = scene->r.view<TransformComponent, SpriteComponent>();

      for(auto entity : view) {
        const auto spriteComponent = view.get<SpriteComponent>(entity);
        const auto transformComponent = view.get<TransformComponent>(entity);

        const int size = 48;
        const int spriteX = spriteComponent.xIndex * size;
        const int spriteY = spriteComponent.yIndex * size;

        SDL_Rect clip = { spriteX, spriteY, size, size };
        
        Texture* texture = TextureManager::GetTexture(spriteComponent.name, spriteComponent.shader.name);

        texture->render(
          transformComponent.position.x,
          transformComponent.position.y,
          300,
          300,
          &clip
        );
      }
    }
};

class SpriteUpdateSystem : public UpdateSystem {
  public:
    void run(float dT) {
      auto view = scene->r.view<SpriteComponent>();

      Uint32 current = SDL_GetTicks();

      for(auto entity : view) {
        auto& sprite = view.get<SpriteComponent>(entity);

        if (sprite.animationDuration > 0) {
          float adT = (current - sprite.lastUpdate) / 1000.0f;
          float animationDurationSeconds = sprite.animationDuration/1000.0f;
          float afps = animationDurationSeconds/60.0f;
          int framesToUpdate = adT/afps;

          if (framesToUpdate > 0) {
            sprite.xIndex += framesToUpdate;
            sprite.xIndex %= sprite.animationFrames;
            sprite.lastUpdate = current;
          }
        }
      }
    }
};

class TilemapSetupSystem : public SetupSystem {
  public:
    TilemapSetupSystem(SDL_Renderer* renderer, SDL_Window* window)
      : renderer(renderer), window(window) { }

    ~TilemapSetupSystem() {
      TextureManager::UnloadTexture("Tiles/Grass.png");
      TextureManager::UnloadTexture("Tiles/Water.png");
    }

  void run() {
      Texture* grassTexture = TextureManager::LoadTexture("Tiles/Grass.png", renderer, window);
      Texture* waterTexture = TextureManager::LoadTexture("Tiles/Water.png", renderer, window);

      // A simple 2x2 map
      int map[] = {
          0, 0,
          1, 0
      };

      // Add the TilemapComponent to the world entity
      auto& tilemap = scene->world->get<TilemapComponent>();
      tilemap.mapWidth = 2;
      tilemap.mapHeight = 2;
      tilemap.tileSize = 16; // Set the tile size here

      for(int i = 0; i < sizeof(map) / sizeof(int); i++) {
          if(map[i] == 0) {
              tilemap.tiles.push_back(grassTexture);
          } else if(map[i] == 1) {
              tilemap.tiles.push_back(waterTexture);
          }
      }
  }

  private:
    SDL_Renderer* renderer;
    SDL_Window* window;
};

class TilemapRenderSystem : public RenderSystem {
  public:
    void run(SDL_Renderer* renderer) {
      auto& tilemap = scene->world->get<TilemapComponent>();

      for(int y = 0; y < tilemap.mapHeight; y++) {
        for(int x = 0; x < tilemap.mapWidth; x++) {
          Texture* tileTexture = tilemap.tiles[y * tilemap.mapWidth + x];

          const int size = tilemap.tileSize;
          SDL_Rect clip = { 0, 0, size, size };

          tileTexture->render(
            x * size,
            y * size,
            size,
            size,
            &clip
          );
        }
      }
    }
};



class PerlinTilemapSetupSystem : public SetupSystem {
public:
  PerlinTilemapSetupSystem(SDL_Renderer* renderer, SDL_Window* window)
    : renderer(renderer), window(window) { }

  ~PerlinTilemapSetupSystem() {
    TextureManager::UnloadTexture("Tiles/Grass.png");
    TextureManager::UnloadTexture("Tiles/Water.png");
  }

  void run() {
    Texture* grassTexture = TextureManager::LoadTexture("Tiles/Grass.png", renderer, window);
    Texture* waterTexture = TextureManager::LoadTexture("Tiles/Water.png", renderer, window);

    // Add the TilemapComponent to the world entity
    auto& tilemap = scene->world->get<TilemapComponent>();
    tilemap.mapWidth = 50;
    tilemap.mapHeight = 38;
    tilemap.tileSize = 16;
    // Initialize the Perlin noise generator
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    float scale = 2.0f; // Increase or decrease this value to change the "zoom level" of the noise
    float waterThreshold = 0.3f; // Increase this value to get more water, or decrease it for less water

    // Generate a random offset
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_real_distribution<> distr(-100000.0, 100000.0); // define the range
    float offsetX = distr(gen);
    float offsetY = distr(gen);

    for (int y = 0; y < tilemap.mapHeight; ++y) {
      for (int x = 0; x < tilemap.mapWidth; ++x) {
        // Generate a noise value between 0.0 and 1.0
        float height = noise.GetNoise((static_cast<float>(x) + offsetX) * scale, (static_cast<float>(y) + offsetY) * scale) * 0.5f + 0.5f;

        // Decide the tile type based on the height value
        if (height < waterThreshold) {
          // Set to water if the height is less than the threshold
          tilemap.tiles.push_back(waterTexture);
        } else {
          // Set to grass otherwise
          tilemap.tiles.push_back(grassTexture);
        }
      }
    }
  }

private:
  SDL_Renderer* renderer;
  SDL_Window* window;
};