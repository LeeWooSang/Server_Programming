#include "stdafx.h"
#include "Player.h"
#include "Renderer.h"

Player::Player()
	: GameObject(), m_HpBar_width(0), m_HpBar_height(0), m_Speed(0), m_TextureID(0)
{
}

Player::~Player()
{
}

void Player::ProcessInput()
{
	if (GetAsyncKeyState(VK_RIGHT) & 0x0001)
		m_X += 100;
	//if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	//	//m_X += m_Speed * m_ElapsedTime;

	if (GetAsyncKeyState(VK_LEFT) & 0x0001)
		m_X -= 100;
	//if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	//	m_X -= m_Speed * m_ElapsedTime;

	if (GetAsyncKeyState(VK_UP) & 0x0001)
		m_Y += 100;
	//if (GetAsyncKeyState(VK_UP) & 0x8000)
	//	m_Y += m_Speed * m_ElapsedTime;

	if (GetAsyncKeyState(VK_DOWN) & 0x0001)
		m_Y -= 100;
	//if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	//	m_Y -= m_Speed * m_ElapsedTime;
}

bool Player::Initialize(Renderer* pRenderer)
{
	m_X = -50.f;
	m_Y = -350.f;
	m_Size = 100.f;

	m_Color[0] = 1.f;
	m_Color[1] = 1.f;
	m_Color[2] = 1.f;
	m_Color[3] = 1.f;

	m_BuildLevel = 0.1f;
	
	m_HpBar_width = m_Size;
	m_HpBar_height = m_Size / 10.f;

	m_Speed = 100.f;
	m_TextureID = pRenderer->CreatePngTexture("./Textures/PNGs/RedKingTower.png");

	return true;
}

void Player::Update(float elapsedTime)
{
	float elapsedTimeInSecond = elapsedTime * 0.001f;
	m_ElapsedTime = elapsedTimeInSecond;

	int size = 350;

	if (m_X >= size)
		m_X = size;
	else if (m_X <= -size)
		m_X = -size;
	if (m_Y >= size)
		m_Y = size;
	else if (m_Y <= -size)
		m_Y = -size;
}

void Player::Render(Renderer* pRenderer)
{
	int i = 0;
	pRenderer->DrawTexturedRect(m_X, m_Y, m_Z, m_Size, m_Color[i++], m_Color[i++], m_Color[i++], m_Color[i++], m_TextureID, m_BuildLevel);
}
