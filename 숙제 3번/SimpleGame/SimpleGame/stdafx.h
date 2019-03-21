#pragma once
// inet_addr 오류 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <map>
#include <list>
#include <WinSock2.h>
#include <Windows.h>
using namespace std;

#define WIDTH 800
#define HEIGHT 800

#define SERVER_IP "127.0.0.1"
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
// 서버에서 클라에게 좌표 값만 보냄
struct SC_InitPacket
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position = { 0 };
};
struct SC_MovePacket
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position[2] = { 0 };
};

struct SC_MovePacket2
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position = { 0 };
};

// 클라에서 서버에게 어떤 키인지만 보냄
struct CS_MovePacket
{
	byte m_PlayerID = 0;
	byte m_Key = KEY_IDLE;
};

#pragma pack()
