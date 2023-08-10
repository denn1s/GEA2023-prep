#include <print.h>

#include "Pong.h"
#include "Systems.h"

#include "ECS/Entity.h"

#include "Game/Graphics/PixelShader.h"


Pong::Pong(const char* name, int width, int height)
  : Game(name, width, height)
{
  Scene* gameplayScene = createGameplayScene();
  setScene(gameplayScene);
}

Pong::~Pong() {
  
}

Uint32 fragment(Uint32 currentColor)
{
  if (currentColor == 0) {
    return currentColor;
  }

  Uint8 red = (currentColor >> 16) & 0xff;
  Uint8 green = (currentColor >> 8) & 0xff;
  Uint8 blue = currentColor  & 0xff;
  
  std::cout << "r: " <<  (int)red << " g: " <<  (int)green << " b: " <<  (int)blue << std::endl;

  return currentColor;
}

Scene* Pong::createGameplayScene() {
  Scene* scene = new Scene("GAMEPLAY SCENE");

  Entity white = scene->createEntity("cat1", 100, 100);
  white.addComponent<SimpleSpriteComponent>("Sprites/Cat/1.png");

  Entity black = scene->createEntity("cat2", 300, 100);
  black.addComponent<SimpleSpriteComponent>(
    "Sprites/Cat/1.png", 
    PixelShader{ [](Uint32 pixel) -> Uint32 { return (pixel == 0xF3F2C0) ? 0xD2B48C : pixel; }, "sampleShader" }
    // PixelShader{ fragment, "sampleShader" }
  );

  Entity anim = scene->createEntity("cat3", 500, 100);
  anim.addComponent<SpriteComponent>(
    "Sprites/Cat/SpriteSheet.png",
    8, // total animationFrames
    5000, // animation duration millis
    0, // x index for this sprite
    0 // y index for this sprite
  );


  // scene->addSetupSystem(new TilemapSetupSystem(renderer, window));
  scene->addSetupSystem(new PerlinTilemapSetupSystem(renderer, window));
  scene->addRenderSystem(new TilemapRenderSystem());


  scene->addSetupSystem(new SimpleSpriteSetupSystem(renderer, window));
  scene->addRenderSystem(new SimpleSpriteRenderSystem());
  scene->addSetupSystem(new SpriteSetupSystem(renderer, window));
  scene->addRenderSystem(new SpriteRenderSystem());
  scene->addUpdateSystem(new SpriteUpdateSystem());

  return scene;
}