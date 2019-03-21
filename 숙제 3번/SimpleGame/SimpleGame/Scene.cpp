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
	auto iter = m_PlayerList.find(m_id);
	if (iter != m_PlayerList.end())
		return ((*iter).second)->ProcessInput();
	else
		return 0;
}

bool Scene::Initialize(const SC_MovePacket& sc_Packet)
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

	// 내 자신을 생성한다.
	Player* pPlayer = new Player;
	if (!pPlayer->Initialize(m_pRenderer))
		return false;
	pPlayer->setID(sc_Packet.m_PlayerID);
	pPlayer->setPosition(sc_Packet.m_Position[pPlayer->getID()]);
	m_PlayerList.emplace(pPlayer->getID(), pPlayer);
	m_id = pPlayer->getID();

	return true;
}

void Scene::PacketProcess(Network& network)
{
	CS_MovePacket cs_packet;
	byte key = Scene::ProcessInput();
//	if (key != KEY_IDLE)
//	{
		cs_packet.m_Key = key;
		network.setCSPacket(cs_packet);
		network.SendPacket();
	//}

	network.RecvPacket(m_PlayerList);

	if (m_PlayerList.size() < network.getSCPacket().m_ClientSize)
	{
		byte NewPlayerSize = network.getSCPacket().m_ClientSize - m_PlayerList.size();
		for (int i = 0; i < NewPlayerSize; ++i)
		{
			Player* pPlayer = new Player;
			if (!pPlayer->Initialize(m_pRenderer))
				return;
			// 먼저 들어온 플레이어 정보를 어떻게 저장함?
			m_PlayerList.emplace(3, pPlayer);
		}
	}
	//// 플레이어 생성
	//for (int i = m_PlayerList.size(); i < network.getSCPacket().m_ClientSize; ++i)
	//{
	//	Player* pPlayer = new Player;
	//	if (!pPlayer->Initialize(m_pRenderer))
	//		return;
	//	// 먼저 들어온 플레이어 정보를 어떻게 저장함?
	//	m_PlayerList.emplace(0, pPlayer);
	//}

	int i = 0;
	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); ++iter)
	{
		(*iter).second->setID(network.getSCPacket().m_PlayerID);
		(*iter).second->setPosition(network.getSCPacket().m_Position[i++]);
		
		//cout << (*iter).second->getPosition().m_X << ", " << (*iter).second->getPosition().m_Y << endl;
	}
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