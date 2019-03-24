#include "stdafx.h"
#include "Network.h"
#include "Player.h"
#include "Scene.h"

Network::Network() : m_cs_packet{ 0 }, m_sc_packet{ 0 }, m_socketInfo{nullptr}
{
}

Network::~Network()
{
	closesocket(m_Server_socket);
	WSACleanup();
}

void Network::err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void Network::err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	cout << msg << (char*)lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}

int Network::recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;

	char *ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, ptr, left, flags);

		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;

		left -= received;
		ptr += received;
	}
	return (len - left);
}

bool Network::Initialize()
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	m_Server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_Server_socket == INVALID_SOCKET)
	{
		err_quit("socket( )");
		return false;
	}

	SOCKADDR_IN server_addr;
	//	connect
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);
	int retval = connect(m_Server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (retval == SOCKET_ERROR)
	{
		err_quit("connect( )");
		return false;
	}

	return true;
}

// 내 자신의 정보를 받아옴
void Network::Recv_InitPacket(Scene* pScene)
{
	do
	{
		int retval = recvn(m_Server_socket, (char*)&m_scInitPacket, sizeof(SC_InitPacket), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recvn( )");
			return;
		}

		if (!pScene->PlayerCreate(Scene::InitPacket, m_scInitPacket, m_scUpdatePacket))
			return;

	} while (m_scInitPacket.m_RemainPacket);

	// Non_Blocking
#ifdef Non_Blocking
	// 넌블러킹으로 전환
	u_long on = 1;
	int optval = ioctlsocket(m_Server_socket, FIONBIO, &on);
	if (optval == SOCKET_ERROR)
	{
		err_quit("ioctlsocket( )");
		return;
	}
#endif 
}

void Network::Recv_UpdatePacket(Scene* pScene, map<byte, Player*>& PlayerList)
{
	do
	{
		int retval = recvn(m_Server_socket, (char*)&m_scUpdatePacket, sizeof(m_scUpdatePacket), 0);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}
			else
			{
				cout << "Error - Fail WSASend(error_code : " << WSAGetLastError() << ")" << endl;
				err_display("recvn( )");
				return;
			}
		}

		auto iter = PlayerList.find(m_scUpdatePacket.m_PlayerID);
		if (iter != PlayerList.end())
		{
			(*iter).second->setPosition(m_scUpdatePacket.m_Position);
		}
		// 플레이어 새로 들어왔으니, 플레이어 생성해줌
		else
		{
			pScene->PlayerCreate(Scene::UpdatePacket, m_scInitPacket, m_scUpdatePacket);
		}

	} while (m_scUpdatePacket.m_RemainPacket);
	
}

void Network::SendPacket()
{
	// 데이터 보내기
	int retval = send(m_Server_socket, (char*)&m_cs_packet, sizeof(m_cs_packet), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("send( )");
		return;
	}
}