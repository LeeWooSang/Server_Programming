#include "stdafx.h"
#include "Player.h"
#include "Renderer.h"

Player::Player()
	: GameObject(), m_HpBar_width(0), m_HpBar_height(0), m_Speed(0), m_TextureID(0), m_ID(0), m_Check(false)
{
}

Player::~Player()
{
}

 int Player::ProcessInput()
{
	 if (GetAsyncKeyState(VK_RIGHT) & 0x0001)			
		 return KEY_RIGHT;
	 else if (GetAsyncKeyState(VK_LEFT) & 0x0001)		return KEY_LEFT;
	 else if (GetAsyncKeyState(VK_UP) & 0x0001)		return KEY_UP;
	 else if (GetAsyncKeyState(VK_DOWN) & 0x0001)	return KEY_DOWN;
	 else																		return KEY_IDLE;
}

 int Player::ProcessInput(int key, int x, int y)
 {
	 if (key == GLUT_KEY_RIGHT)
		 return KEY_RIGHT;
	 else if (key == GLUT_KEY_LEFT)
		 return KEY_LEFT;
	 else if (key == GLUT_KEY_UP)
		 return KEY_UP;
	 else if (key == GLUT_KEY_DOWN)
		 return KEY_DOWN;
	 else
		 return KEY_IDLE;
 }

bool Player::Initialize(Renderer* pRenderer)
{
	m_Position.m_X = 0.f;
	m_Position.m_Y = 0.f;
	m_Size = 100.f;

	m_Color[0] = 1.f;
	m_Color[1] = 1.f;
	m_Color[2] = 1.f;
	m_Color[3] = 1.f;

	m_BuildLevel = 0.1f;
	
	m_HpBar_width = m_Size;
	m_HpBar_height = m_Size / 10.f;

	m_Speed = 100.f;

	//random_device seed;
	//default_random_engine dre(seed());
	//uniform_int_distribution<int> uid(Pawn, King);
	//string filename = "./Textures/PNGs/";
	//switch (uid(dre))
	//{
	//case Pawn:	filename += "Chess_Pawn.png";		break;
	//case Rookie:	filename += "Chess_Rookie.png";		break;
	//case Knight:	filename += "Chess_Knight.png";		break;
	//case Bishop: filename += "Chess_Bishop.png";		break;
	//case Queen: filename += "Chess_Queen.png";		break;
	//case King:		filename += "Chess_King.png";			break;
	//}

	//m_TextureID = pRenderer->CreatePngTexture(const_cast<char*>(filename.c_str()));
	m_TextureID = pRenderer->CreatePngTexture("./Textures/PNGs/Chess_Knight.png");
	return true;
}

bool Player::Initialize(Renderer* pRenderer, int TextureID)
{
	m_Position.m_X = 0.f;
	m_Position.m_Y = 0.f;
	m_Size = 100.f;

	m_Color[0] = 1.f;
	m_Color[1] = 1.f;
	m_Color[2] = 1.f;
	m_Color[3] = 1.f;

	m_BuildLevel = 0.1f;

	m_HpBar_width = m_Size;
	m_HpBar_height = m_Size / 10.f;

	m_Speed = 100.f;

	string filename = "./Textures/PNGs/";
	switch (TextureID)
	{
	case Pawn:	filename += "Chess_Pawn.png";		break;
	case Rookie:	filename += "Chess_Rookie.png";		break;
	case Knight:	filename += "Chess_Knight.png";		break;
	case Bishop: filename += "Chess_Bishop.png";		break;
	case Queen: filename += "Chess_Queen.png";		break;
	case King:		filename += "Chess_King.png";			break;
	}

m_TextureID = pRenderer->CreatePngTexture(const_cast<char*>(filename.c_str()));

	return true;
}
void Player::Update(float elapsedTime, float x, float y)
{
	//m_X = x;
	//m_Y = y;

	float elapsedTimeInSecond = elapsedTime * 0.001f;
	m_ElapsedTime = elapsedTimeInSecond;

	int size = 350;

	if (m_Position.m_X >= size)
		m_Position.m_X = size;
	else if (m_Position.m_X <= -size)
		m_Position.m_X = -size;
	if (m_Position.m_Y >= size)
		m_Position.m_Y = size;
	else if (m_Position.m_Y <= -size)
		m_Position.m_Y = -size;
}

void Player::Render(Renderer* pRenderer)
{
	//cout << "플레이어 : " << m_Position.m_X << ", " << m_Position.m_Y << endl;
	int i = 0;
	pRenderer->DrawTexturedRect
	(
		m_Position.m_X,
		m_Position.m_Y,
		m_Z, 
		m_Size, 
		m_Color[i++], 
		m_Color[i++], 
		m_Color[i++], 
		m_Color[i++], 
		m_TextureID, 
		m_BuildLevel
	);
}
