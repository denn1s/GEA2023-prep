#include <print.h>

#include "Pong.h"
#include "Systems.h"

#include "ECS/Entity.h"

Pong::Pong(const char* name, int width, int height)
  : Game(name, width, height)
{
  Scene* gameplayScene = createGameplayScene();
  setScene(gameplayScene);
}

Pong::~Pong() {
  
}

Scene* Pong::createGameplayScene() {
  Scene* scene = new Scene("GAMEPLAY SCENE");

  Entity ball = scene->createEntity("BALL", 10, 10);
  ball.addComponent<SizeComponent>(20, 20);
  ball.addComponent<SpeedComponent>(200, 200);
  ball.addComponent<ColliderComponent>(false, 0);

  Entity paddle = scene->createEntity("PLAYER 1", (screen_width/2) - 50, screen_height - 20);
  paddle.addComponent<SizeComponent>(100, 20);
  paddle.addComponent<SpeedComponent>(0, 0);
  paddle.addComponent<PlayerComponent>(200);

  
  scene->addSetupSystem(new HelloWorldSystem());
  scene->addRenderSystem(new RectRenderSystem());
  scene->addUpdateSystem(new BounceUpdateSystem());
  scene->addUpdateSystem(new MovementUpdateSystem(screen_width, screen_height));
  scene->addEventSystem(new PlayerInputSystem());

  scene->addUpdateSystem(new CollisionDetectionUpdateSystem());

  return scene;
}