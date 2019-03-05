#include "stdafx.h"
#include "Scene.h"
#include "Renderer.h"
#include "ChessBoard.h"
#include "Player.h"

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

void Scene::ProcessInput()
{
	auto iter = m_GameObjectMap.find("Player");
	if (iter != m_GameObjectMap.end())
		dynamic_cast<Player*>((*iter).second)->ProcessInput();
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
			m_GameObjectMap.emplace("ChessBoard" + to_string(m_GameObjectMap.size()), pChessBoard);
			
			width += 100.f;

			++order;
		}
		width = 0.f;
		height += 100.f;
		++order;
	}

	Player* pPlayer = new Player;
	if (!pPlayer->Initialize(m_pRenderer))
		return false;
	m_GameObjectMap.emplace("Player", pPlayer);
}

void Scene::Update(float elapsedTime)
{
	Scene::ProcessInput();

	for (auto iter = m_GameObjectMap.begin(); iter != m_GameObjectMap.end(); ++iter)
	{
		(*iter).second->Update(elapsedTime);
	}
}

void Scene::Render()
{
	for (auto iter = m_GameObjectMap.begin(); iter != m_GameObjectMap.end(); ++iter)
	{
		(*iter).second->Render(m_pRenderer);
	}
}

void Scene::Release()
{
	for (auto iter = m_GameObjectMap.begin(); iter != m_GameObjectMap.end(); )
	{
		delete (*iter).second;
		iter = m_GameObjectMap.erase(iter);
	}
	m_GameObjectMap.clear();
}