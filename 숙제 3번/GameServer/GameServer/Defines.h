#pragma once

#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <map>
using namespace std;

enum SERVER_INFO { SERVER_PORT = 9000, BUFSIZE = 1024 };

enum KEY 
{
	KEY_IDLE		= 0x00,
	KEY_RIGHT	= 0x01,
	KEY_LEFT		= 0x02,
	KEY_UP			= 0x04,
	KEY_DOWN	= 0x08
};

struct Position
{
	float m_X = 0.f;
	float m_Y = 0.f;
};

#pragma pack(1)

// �������� Ŭ�󿡰� ��ǥ ���� ����
struct SC_MovePacket
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position[2] = { {-350, -350 }, {-250, -350} };
};

// Ŭ�󿡼� �������� � Ű������ ����
struct CS_MovePacket
{
	byte m_PlayerID = 0;
	byte m_Key = KEY_IDLE;
};
#pragma pack()
