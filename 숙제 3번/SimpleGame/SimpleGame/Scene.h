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
	void PacketProcess(Network& network);

	void Update(float, Network&);
	void Render();
	void Release();

	// 맨 처음에 플레이어 생성
	bool PlayerCreate(byte, const SC_InitPacket&, const SC_UpdatePacket&);
	// 나간 플레이어
	void LeavePlayerDestroy(const SC_UpdatePacket&);

	map<byte, Player*>& getPlayerList()	{ return m_PlayerList; }

private:
	Renderer*								m_pRenderer;
	map<string, GameObject*> m_GameObjectList;
	map<byte, Player*>				m_PlayerList;

	float m_SendTime;
};

