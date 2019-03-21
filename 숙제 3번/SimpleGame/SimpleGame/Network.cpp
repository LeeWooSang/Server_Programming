#include "stdafx.h"
#include "Network.h"
#include "Player.h"

Network::Network() : m_cs_packet{ 0 }, m_sc_packet{ 0 }, m_socketInfo{nullptr}
{
}

Network::~Network()
{
	if (m_socketInfo)
		delete m_socketInfo;

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

	//m_Server_socket = socket(AF_INET, SOCK_STREAM, 0);
	m_Server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Server_socket == INVALID_SOCKET)
	{
		err_quit("socket( )");
		return 0;
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

	m_socketInfo = new SOCKETINFO;

	return true;
}

void Network::Update()
{
}

// 내 자신의 정보를 받아옴
void Network::InitRecvPacket()
{
	int retval = recvn(m_Server_socket, (char*)&m_sc_packet, sizeof(m_sc_packet), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("recvn( )");
		return;
	}
}

void Network::RecvPacket(const map<byte, Player*>& PlayerList)
{
	////데이터 받기
	//for (auto iter = PlayerList.begin(); iter != PlayerList.end(); ++iter)
	//{
	//	int retval = recvn(m_Server_socket, (char*)&m_sc_packet, sizeof(m_sc_packet), 0);
	//	if (retval == SOCKET_ERROR)
	//	{
	//		err_display("recvn( )");
	//		return;
	//	}
	//	cout << "받음" << endl;
	//}

	//for (int i = 0; i < 2; ++i)
	//{
		int retval = recvn(m_Server_socket, (char*)&m_sc_packet, sizeof(m_sc_packet), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recvn( )");
			return;
		}

		cout << m_sc_packet.m_Position[0].m_X << ", " << m_sc_packet.m_Position[0].m_Y << endl;
//	}
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

