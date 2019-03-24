#include "Defines.h"

struct SOCKETINFO
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF					m_dataBuffer;

	SOCKET m_socket;

	SC_InitPacket		m_scInitPacket;
	SC_UpdatePacket	m_scUpdatePacket;
	CS_MovePacket	m_csPacket;

	ClientInfo	m_ClientInfo;

	int								m_receiveBytes;
	int								m_sendBytes;
};

bool CtrlHandler(DWORD);
void err_quit(const char*);
void err_display(const char*);
bool Initialize();
void Release();
void PacketProcess(SOCKET& client_socket, CS_MovePacket& csPacket);

// 콜백함수 정의
void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

// ★★★★★소켓으로 어떤 클라이언트에서 왔는지 알 수 있게 해야함
map <SOCKET, SOCKETINFO> g_clientList;

int main()
{
	if (!Initialize())
		return 0;
}

bool CtrlHandler(DWORD CtrlType)
{
	switch (CtrlType)
	{
		// Handle the CTRL+C signal. 
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT: // CTRL+CLOSE: confirm! that the user wants to exit. 
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	default:
		Release();
		return false;
	}
}

void err_quit(const char* msg)
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

void err_display(const char* msg)
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

bool Initialize()
{
	int retval = 0;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	// Socket( )
	// 대기 소켓 생성
	SOCKET listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_socket == INVALID_SOCKET)
	{
		err_quit("socket( )");
		return false;
	}

	// bind( )
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	// namespace std에 있는 bind 함수와 Winsock2에 있는 bind 함수와 겹쳐서 구분
	retval = ::bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (retval == SOCKET_ERROR)
	{
		err_quit("bind( )");
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}

	// listen( )
	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		err_quit("listen( )");
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}
	cout << "대기 소켓 생성 완료" << endl;

	SOCKET client_socket;
	SOCKADDR_IN client_addr;
	int addrlen = sizeof(client_addr);
	ZeroMemory(&client_addr, addrlen);
	DWORD flags = 0;
	static byte playerID = 0;

	Position position[10];
	float termX = 0.f;
	for (int i = 0; i < 10; ++i)
	{
		position[i].m_X = -350.f + termX;
		position[i].m_Y = -350.f;
		termX += 100.f;
	}
	termX = 0;
	for (int i = 8; i < 10; ++i)
	{
		position[i].m_X = -350.f + termX;
		position[i].m_Y = -250.f;
		termX += 100.f;
	}

	while (true)
	{
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true);
		// accept( )
		client_socket = accept(listen_socket, (SOCKADDR*)&client_addr, &addrlen);
		if (client_socket == INVALID_SOCKET)
		{
			err_display("accept( )");
			break;
		}
		cout << "\n[ TCP 서버 ] 클라이언트 접속 - IP : " << inet_ntoa(client_addr.sin_addr)
			<< ", 포트 번호 : " << ntohs(client_addr.sin_port) << endl;

		// 클라이언트 정보를 0으로 초기화
		g_clientList.emplace(client_socket, SOCKETINFO{});
		auto iter = g_clientList.find(client_socket);
		if (iter != g_clientList.end())
		{
			// 클라이언트 정보를 0으로 초기화
			ZeroMemory(&((*iter).second), sizeof(SOCKETINFO));
			(*iter).second.m_socket = client_socket;

			(*iter).second.m_ClientInfo.m_ID = playerID++;
			(*iter).second.m_ClientInfo.m_Position = position[(*iter).second.m_ClientInfo.m_ID];
			(*iter).second.m_ClientInfo.m_LeavePlayerID = -1;

			(*iter).second.m_scInitPacket.m_ClientSize = g_clientList.size();
			(*iter).second.m_dataBuffer.len = sizeof(SC_InitPacket);
			(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scInitPacket);
			(*iter).second.m_overlapped.hEvent = (HANDLE)(*iter).second.m_socket;

			byte count = g_clientList.size() - 1;
			for (auto iter2 = g_clientList.begin(); iter2 != g_clientList.end(); ++iter2)
			{
				(*iter).second.m_scInitPacket.m_PlayerID = (*iter2).second.m_ClientInfo.m_ID;
				(*iter).second.m_scInitPacket.m_Position = (*iter2).second.m_ClientInfo.m_Position;
				(*iter).second.m_scInitPacket.m_RemainPacket = count--;

				if ((*iter).second.m_socket == (*iter2).second.m_socket)
					(*iter).second.m_scInitPacket.m_Check = true;
				else
					(*iter).second.m_scInitPacket.m_Check = false;

				if (WSASend((*iter).second.m_socket, &(*iter).second.m_dataBuffer, 1, NULL, 0,
					&((*iter).second.m_overlapped), send_callback) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Error - IO pending Failure\n");
						closesocket(client_socket);
						break;
					}
				}
			}
		}
		else
		{
			closesocket(client_socket);
			break;
		}
	}

	closesocket(listen_socket);

	WSACleanup();

	return true;
}

void Release()
{
	for (auto iter = g_clientList.begin(); iter != g_clientList.end(); )
	{
		closesocket((*iter).second.m_socket);
		iter = g_clientList.erase(iter);
	}
	g_clientList.clear();

	//closesocket(listen_socket);

	//WSACleanup();
}

// 패킷처리를 recv_callback에서 담당한다.
// recv_callback 1개의 함수에서 소켓마다 다르게 처리한다.
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	// 인덱스를 넣어 줌. 
	// 람다를 사용할 수 도 있음
	SOCKET client_socket = reinterpret_cast<int>(overlapped->hEvent);
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	// 클라이언트에서 closesocket( )을 호출한 경우
	if (dataBytes == 0)
	{
		auto iter = g_clientList.find(client_socket);
		if (iter != g_clientList.end())
		{
			// 나간 플레이어 ID를 접속해있는 플레이어에게 저장시켜줌
			for (auto iter2 = g_clientList.begin(); iter2 != g_clientList.end(); ++iter2)
				(*iter2).second.m_ClientInfo.m_LeavePlayerID = (*iter).second.m_ClientInfo.m_ID;

			cout << "클라 게임 종료" << endl;
			// 해당하는 클라의 소켓을 서버에서도 지움.
			closesocket((*iter).second.m_socket);
			iter = g_clientList.erase(iter);
		}
		return;
	}

	auto iter = g_clientList.find(client_socket);
	if (iter != g_clientList.end())
	{
		//cout << "TRACE - Receive : KEY_LEFT - (" << dataBytes << ") bytes)" << endl;
		PacketProcess(client_socket, g_clientList[client_socket].m_csPacket);

		// 패킷처리가 끝났으면, 다시 recv( )를 해야함
		// 그렇지 않으면, send( )하는 동안 recv( )를 할 수 없어서 지연 됨.

		byte playerID = g_clientList.size() - 1;
		(*iter).second.m_scUpdatePacket.m_ClientSize = g_clientList.size();
		(*iter).second.m_scUpdatePacket.m_LeavePlayerID = (*iter).second.m_ClientInfo.m_LeavePlayerID;
		(*iter).second.m_dataBuffer.len = sizeof(SC_UpdatePacket);
		(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scUpdatePacket);
		ZeroMemory(&((*iter).second.m_overlapped), sizeof(WSAOVERLAPPED));
		(*iter).second.m_overlapped.hEvent = (HANDLE)client_socket;

		byte count = g_clientList.size() - 1;
		for (auto iter2 = g_clientList.begin(); iter2 != g_clientList.end(); ++iter2)
		{
			(*iter).second.m_scUpdatePacket.m_PlayerID = (*iter2).second.m_ClientInfo.m_ID;
			(*iter).second.m_scUpdatePacket.m_Position = (*iter2).second.m_ClientInfo.m_Position;
			(*iter).second.m_scUpdatePacket.m_RemainPacket = count--;

			if (WSASend(client_socket, &(*iter).second.m_dataBuffer, 1, &dataBytes, 0, &((*iter).second.m_overlapped), send_callback) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
					cout << "Error - Fail WSASend(error_code : " << WSAGetLastError() << ")" << endl;
			}
		}
	}
}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_socket = reinterpret_cast<int>(overlapped->hEvent);
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	// 클라이언트에서 closesocket( )을 호출한 경우
	if (dataBytes == 0)
	{
		//auto iter = g_clientList.find(client_socket);
		//if (iter != g_clientList.end())
		//{
		//	// 해당하는 클라의 소켓을 서버에서도 지움.
		//	closesocket((*iter).second.m_socket);
		//	iter = g_clientList.erase(iter);
		//}
		return;
	}

	auto iter = g_clientList.find(client_socket);
	if (iter != g_clientList.end())
	{
		//cout << "TRACE - Send : (" << dataBytes << " bytes)" << endl;

		// WSASend(응답에 대한)의 콜백일 경우
		//(*iter).second.m_dataBuffer.len = BUFSIZE;
		//(*iter).second.m_dataBuffer.buf = (*iter).second.m_messageBuffer;

		(*iter).second.m_dataBuffer.len = sizeof(CS_MovePacket);
		(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_csPacket);
		ZeroMemory(&((*iter).second.m_overlapped), sizeof(WSAOVERLAPPED));
		(*iter).second.m_overlapped.hEvent = (HANDLE)client_socket;

		if (WSARecv(client_socket, &(*iter).second.m_dataBuffer, 1, &receiveBytes, &flags, &((*iter).second.m_overlapped), recv_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				cout << "Error - Fail WSARecv(error_code : " << WSAGetLastError() << ")" << endl;
			// 클라가 아직 send 안함
		}
	}
}

void PacketProcess(SOCKET& client_socket, CS_MovePacket& csPacket)
{
	auto iter = g_clientList.find(client_socket);
	if (iter != g_clientList.end())
	{
		switch (csPacket.m_Key)
		{
		case KEY_RIGHT:
			(*iter).second.m_ClientInfo.m_Position.m_X += 100;
			break;
		case KEY_LEFT:
			(*iter).second.m_ClientInfo.m_Position.m_X -= 100;
			break;
		case KEY_UP:
			(*iter).second.m_ClientInfo.m_Position.m_Y += 100;
			break;
		case KEY_DOWN:
			(*iter).second.m_ClientInfo.m_Position.m_Y -= 100;
			break;
			// KEY_IDLE
		default:
			break;
		}
	}

}