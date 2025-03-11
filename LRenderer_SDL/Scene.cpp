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

SphereBoundingBox Scene::GetSphereBoudingBox()
{
	sceneSbb.center = Eigen::Vector3f(0, 0, 0);
	sceneSbb.radius = 0;	
	
	for (auto& gameObject : gameObjects.GetValidItems())
	{
		auto meshRenderer = gameObject->GetComponent<MeshRenderer>();
		
		if (!meshRenderer)
		{
			continue;
		}

		auto sbb = meshRenderer->GetSphereBoundingBox();
		sceneSbb.center += sbb.center / gameObjects.count;
	}

	for (auto& gameObject : gameObjects.GetValidItems())
	{
		auto meshRenderer = gameObject->GetComponent<MeshRenderer>();

		if (!meshRenderer)
		{
			continue;
		}

		auto sbb = meshRenderer->GetSphereBoundingBox();
		sceneSbb.radius = std::max(sceneSbb.radius, 
			(sceneSbb.center - sbb.center).norm() + sbb.radius);
	}

    return sceneSbb;
}
