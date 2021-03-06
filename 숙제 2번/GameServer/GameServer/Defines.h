#pragma once

#include <iostream>
#include <WinSock2.h>
using namespace std;

enum SERVER_INFO { PORT = 9000 };

enum KEY 
{
	KEY_IDLE		= 0x00,
	KEY_RIGHT	= 0x01,
	KEY_LEFT		= 0x02,
	KEY_UP			= 0x04,
	KEY_DOWN	= 0x08
};

#pragma pack(1)
// 서버에서 클라에게 좌표 값만 보냄
struct SC_MovePacket
{
	float	m_X = 0.f;
	float m_Y = 0.f;
};
// 클라에서 서버에게 어떤 키인지만 보냄
struct CS_MovePacket
{
	byte m_Key = KEY_IDLE;
};
#pragma pack()
