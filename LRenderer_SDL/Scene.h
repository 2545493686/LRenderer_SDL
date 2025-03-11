#pragma once

#include <vector>
#include "Pool.h"
#include "GameObject.h"
#include "BoundingBox.h"
#include "Transform.h"
#include "MeshRenderer.h"

// TODO: 相机
class Scene
{
public:
	void AddGameObject(GameObject* gameObject);
	void RemoveGameObject(GameObject* gameObject);
	
	SphereBoundingBox GetSphereBoudingBox();
	Pool<GameObject *> gameObjects;

private:

	SphereBoundingBox sceneSbb;
};

