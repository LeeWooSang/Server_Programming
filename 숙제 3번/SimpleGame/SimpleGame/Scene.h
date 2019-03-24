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
	int ProcessInput(int, int, int);

	bool Initialize();
	void PacketProcess(Network& network);

	void Update(float, Network&);
	void Render();
	void Release();

	// �� ó���� �÷��̾� ����
	bool PlayerCreate(byte, const SC_InitPacket&, const SC_UpdatePacket&);
	// ���� �÷��̾�
	void LeavePlayerDestroy(const SC_UpdatePacket&);

	map<byte, Player*>& getPlayerList()	{ return m_PlayerList; }

	float getTime()	const { return m_SendTime; }
	void setTime(float time) { m_SendTime = time; }

private:
	Renderer*								m_pRenderer;
	map<string, GameObject*> m_GameObjectList;
	map<byte, Player*>				m_PlayerList;

	float m_SendTime;
};

