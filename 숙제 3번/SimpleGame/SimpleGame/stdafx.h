#pragma once
// inet_addr ���� ����
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
#include <random>
using namespace std;

#define WIDTH 800
#define HEIGHT 800

#define SERVER_IP "127.0.0.1"
#define Non_Blocking

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
struct SC_InitPacket
{
	byte			m_PlayerID = 0;
	Position	m_Position = { 0 };
	// �÷��̾� �ؽ��� ���̵�
	byte			m_TextureID;

	byte			m_RemainPacket = 0;	
	// ������ �ٸ� �÷��̾����� �����ϱ� ����
	bool			m_Check = false;
};

struct SC_UpdatePacket
{
	byte m_PlayerID = 0;
	Position m_Position = { 0 };
	// �÷��̾� �ؽ��� ���̵�
	byte			m_TextureID;
	byte m_RemainPacket = 0;
	char m_LeavePlayerID = -1;
};

// Ŭ�󿡼� �������� � Ű������ ����
struct CS_MovePacket
{
	byte m_Key = KEY_IDLE;
};

#pragma pack()
