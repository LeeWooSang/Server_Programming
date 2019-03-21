#pragma once

class Renderer;
class GameObject;
class Scene
{
public:
	Scene(int, int);
	~Scene();

	void ProcessInput();

	bool Initialize();
	void Update(float);
	void Render();
	void Release();

private:
	Renderer*								m_pRenderer{ nullptr };
	map<string, GameObject*> m_GameObjectMap;
};

