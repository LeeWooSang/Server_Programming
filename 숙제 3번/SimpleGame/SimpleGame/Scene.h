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

	bool Initialize(const SC_MovePacket&);
	void PacketProcess(Network& network);
	void Update(float, Network&);
	void Render();
	void Release();

	const map<byte, Player*>& getPlayerList()	const { return m_PlayerList; }

private:
	Renderer*								m_pRenderer{ nullptr };
	map<string, GameObject*> m_GameObjectList;
	map<byte, Player*>				m_PlayerList;

	byte m_id = 0;
};

