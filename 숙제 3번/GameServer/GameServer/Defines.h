#pragma once

#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <map>
#include <random>
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
	float m_X;
	float m_Y;
};

struct ClientInfo
{
	// 플레이어 ID
	byte			m_ID;
	// 플레이어 위치
	Position	m_Position;
	// 플레이어 텍스쳐 아이디
	byte			m_TextureID;
	// 탈주한 플레이어 ID
	char			m_LeavePlayerID;
};

#pragma pack(1)

struct SC_InitPacket
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position = { -350, -350 };
	byte m_RemainPacket = 0;
	// 나인지 다른 플레이어인지 구분하기 위해
	bool m_Check = false;
};

struct SC_UpdatePacket
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position = { 0 };
	// 클라에게 패킷을 몇번 받아야하는지 알려줌
	byte m_RemainPacket = 0;
	char m_LeavePlayerID = 0;
};

// 클라에서 서버에게 어떤 키인지만 보냄
struct CS_MovePacket
{
	byte m_Key = KEY_IDLE;
};
#pragma pack()
