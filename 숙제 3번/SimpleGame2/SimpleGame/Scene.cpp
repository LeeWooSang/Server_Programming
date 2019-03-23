#include "stdafx.h"
#include "Scene.h"
#include "Renderer.h"
#include "ChessBoard.h"
#include "Player.h"
#include "Network.h"

Scene::Scene(int width , int height)
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
	if (key != KEY_IDLE)
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

	//if (m_PlayerList.size() < network.getSCPacket().m_ClientSize)
	//{
	//	byte NewPlayerSize = network.getSCPacket().m_ClientSize - m_PlayerList.size();
	//	for (int i = 0; i < NewPlayerSize; ++i)
	//	{
	//		Player* pPlayer = new Player;
	//		if (!pPlayer->Initialize(m_pRenderer))
	//			return;
	//		// 먼저 들어온 플레이어 정보를 어떻게 저장함?
	//		m_PlayerList.emplace(3, pPlayer);
	//	}
	//}

	//int i = 0;
	//for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); ++iter)
	//{
	//	(*iter).second->setID(network.getSCPacket().m_PlayerID);
	//	(*iter).second->setPosition(network.getSCPacket().m_Position[i++]);
	//}
}

void Scene::Update(float elapsedTime, Network& network)
{
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

bool Scene::PlayerCreate(byte PacketType, SC_InitPacket& SCInitPacket, SC_UpdatePacket& SCUpdatePacket)
{
	// 플레이어 생성
	Player* pPlayer = new Player;
	if (!pPlayer->Initialize(m_pRenderer))
		return false;

	switch (PacketType)
	{
	case InitPacket:
		pPlayer->setID(SCInitPacket.m_PlayerID);
		pPlayer->setPosition(SCInitPacket.m_Position);
		pPlayer->setCheck(SCInitPacket.m_Check);
		break;

	case UpdatePacket:
		pPlayer->setID(SCUpdatePacket.m_PlayerID);
		pPlayer->setPosition(SCUpdatePacket.m_Position);
		break;

	default:
		return false;
	}

	m_PlayerList.emplace(pPlayer->getID(), pPlayer);

	return true;
}