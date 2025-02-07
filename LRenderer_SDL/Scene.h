#pragma once

#include <vector>
#include "Pool.h"
#include "GameObject.h"

// TODO: 相机
class Scene
{
public:
	void AddGameObject(GameObject* gameObject);
	void RemoveGameObject(GameObject* gameObject);

	std::vector<GameObject*> GetGameObjects();

private:
	Pool<GameObject *> gameObjects;
};

