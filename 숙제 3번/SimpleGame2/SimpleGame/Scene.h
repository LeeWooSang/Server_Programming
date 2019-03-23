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
	// �� ó���� �÷��̾� ����
	bool PlayerCreate(byte, SC_InitPacket&, SC_UpdatePacket&);

	bool Initialize();
	void PacketProcess(Network& network);

	void Update(float, Network&);
	void Render();
	void Release();

	map<byte, Player*>& getPlayerList()	{ return m_PlayerList; }

	enum PacketType { InitPacket, UpdatePacket };

private:
	Renderer*								m_pRenderer{ nullptr };
	map<string, GameObject*> m_GameObjectList;
	map<byte, Player*>				m_PlayerList;
};

