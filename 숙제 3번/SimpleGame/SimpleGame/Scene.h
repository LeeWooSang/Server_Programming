#pragma once

class Renderer;
class GameObject;
class Player;
class Network;
class Scene
{
public:
	Scene(int, int);
	~Scene();

	int ProcessInput();

	bool Initialize();
	void PacketProcessing(Network& network);
	void Update(float, Network&);
	void Render();
	void Release();

private:
	Renderer*								m_pRenderer{ nullptr };
	map<string, GameObject*> m_GameObjectList;
	map<byte, Player*>				m_PlayerList;
};

