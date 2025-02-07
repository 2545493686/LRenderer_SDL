#include "Scene.h"

void Scene::AddGameObject(GameObject *gameObject)
{
	int index = gameObjects.Push(gameObject);
	gameObject->sceneIndex = index;
}

void Scene::RemoveGameObject(GameObject *gameObject)
{
	gameObjects.Remove(gameObject->sceneIndex);
}

std::vector<GameObject *> Scene::GetGameObjects()
{
	std::vector<GameObject *> result;

	for (size_t i = 0; i < gameObjects.data.size(); i++)
	{
		if (gameObjects.valid[i])
		{
			result.push_back(gameObjects.data[i]);
		}
	}
	return result;
}
