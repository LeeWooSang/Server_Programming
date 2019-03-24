#include "stdafx.h"
#include "Scene.h"
#include "Renderer.h"
#include "ChessBoard.h"
#include "Player.h"
#include "Network.h"

Scene::Scene(int width , int height) : m_pRenderer{nullptr}, m_SendTime{0}
{
	 m_pRenderer = new Renderer(width, height);

	if (!m_pRenderer->IsInitialized())
		cout << "Scene::Render could not be initailized!" << endl;
}

Scene::~Scene()
{
	Release();

	if(m_pRenderer)
		delete  m_pRenderer;
}

int Scene::ProcessInput()
{
	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); ++iter)
	{
		if ((*iter).second->getCheck())
			return (*iter).second->ProcessInput();
	}
}

bool Scene::Initialize()
{
	int size = 8;
	float width = 0.f, height = 0.f;
	int order = 0;

	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			ChessBoard* pChessBoard = new ChessBoard;
			if (!pChessBoard->Initialize(m_pRenderer, order % 2, -350.f + width, -350.f + height))
				return false;
			m_GameObjectList.emplace("ChessBoard" + to_string(m_GameObjectList.size()), pChessBoard);
			
			width += 100.f;

			++order;
		}
		width = 0.f;
		height += 100.f;
		++order;
	}

	return true;
}

void Scene::PacketProcess(Network& network)
{
	CS_MovePacket cs_packet;
	byte key = Scene::ProcessInput();
#ifdef Non_Blocking
	if (m_SendTime > 1.f)
	{
		cs_packet.m_Key = key;
		network.setCSPacket(cs_packet);
		network.SendPacket();
	}
#else
	cs_packet.m_Key = key;
	network.setCSPacket(cs_packet);
	network.SendPacket();
#endif
	network.Recv_UpdatePacket(this, m_PlayerList);
	// 탈주한 플레이어가 있는지 확인한다.
	LeavePlayerDestroy(network.getscUpdatePacket());
}

void Scene::Update(float elapsedTime, Network& network)
{
	m_SendTime += elapsedTime * 0.001;
	
	PacketProcess(network);

	// ChessBoard Update;
	for (auto iter = m_GameObjectList.begin(); iter != m_GameObjectList.end(); ++iter)
	{
		(*iter).second->Update(elapsedTime, 0, 0);
	}

	// Player Update;
	for(auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); ++iter)
		(*iter).second->Update(elapsedTime, 0, 0);
}

void Scene::Render()
{
	// ChessBoard Render
	for (auto iter = m_GameObjectList.begin(); iter != m_GameObjectList.end(); ++iter)
	{
		(*iter).second->Render(m_pRenderer);
	}

	// Player Render
	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); ++iter)
	{
		(*iter).second->Render(m_pRenderer);
	}
}

void Scene::Release()
{
	for (auto iter = m_GameObjectList.begin(); iter != m_GameObjectList.end(); )
	{
		delete (*iter).second;
		iter = m_GameObjectList.erase(iter);
	}
	m_GameObjectList.clear();

	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); )
	{
		delete (*iter).second;
		iter = m_PlayerList.erase(iter);
	}
	m_PlayerList.clear();
}

bool Scene::PlayerCreate(byte PacketType, const SC_InitPacket& SCInitPacket, const SC_UpdatePacket& SCUpdatePacket)
{
	// 플레이어 생성
	Player* pPlayer = new Player;
	if (!pPlayer->Initialize(m_pRenderer))
		return false;

	switch (PacketType)
	{
	case Network::InitPacket:
		pPlayer->setID(SCInitPacket.m_PlayerID);
		pPlayer->setPosition(SCInitPacket.m_Position);
		pPlayer->setCheck(SCInitPacket.m_Check);
		break;

	case Network::UpdatePacket:
		pPlayer->setID(SCUpdatePacket.m_PlayerID);
		pPlayer->setPosition(SCUpdatePacket.m_Position);
		break;

	default:
		return false;
	}

	m_PlayerList.emplace(pPlayer->getID(), pPlayer);

	return true;
}

void Scene::LeavePlayerDestroy(const SC_UpdatePacket& SCUpdatePacket)
{
	auto iter = m_PlayerList.find(SCUpdatePacket.m_LeavePlayerID);
	if (iter != m_PlayerList.end())
	{
		delete (*iter).second;
		iter = m_PlayerList.erase(iter);
		cout << (int)SCUpdatePacket.m_LeavePlayerID << "번 플레이어 탈주" << endl;
	}
}