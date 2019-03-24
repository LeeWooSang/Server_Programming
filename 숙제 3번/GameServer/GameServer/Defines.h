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
	// �÷��̾� ID
	byte			m_ID;
	// �÷��̾� ��ġ
	Position	m_Position;
	// �÷��̾� �ؽ��� ���̵�
	byte			m_TextureID;
	// Ż���� �÷��̾� ID
	char			m_LeavePlayerID;
};

#pragma pack(1)

struct SC_InitPacket
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position = { -350, -350 };
	byte m_RemainPacket = 0;
	// ������ �ٸ� �÷��̾����� �����ϱ� ����
	bool m_Check = false;
};

struct SC_UpdatePacket
{
	byte m_PlayerID = 0;
	byte m_ClientSize = 0;
	Position m_Position = { 0 };
	// Ŭ�󿡰� ��Ŷ�� ��� �޾ƾ��ϴ��� �˷���
	byte m_RemainPacket = 0;
	char m_LeavePlayerID = 0;
};

// Ŭ�󿡼� �������� � Ű������ ����
struct CS_MovePacket
{
	byte m_Key = KEY_IDLE;
};
#pragma pack()
