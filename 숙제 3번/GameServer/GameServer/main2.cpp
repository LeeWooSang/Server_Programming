#include "Defines.h"
#include "Player.h"

struct SOCKETINFO
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF					m_dataBuffer;

	SOCKET m_socket;

	char m_messageBuffer[BUFSIZE];

	SC_MovePacket2 m_scPacket;
	CS_MovePacket m_csPacket;

	int								m_receiveBytes;
	int								m_sendBytes;
};

void err_quit(const char*);
void err_display(const char*);
int recvn(SOCKET, char*, int, int);
bool Initialize();
void PacketProcess2(SOCKET& client_socket, CS_MovePacket& csPacket);

// 콜백함수 정의
void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

// ★★★★★소켓으로 어떤 클라이언트에서 왔는지 알 수 있게 해야함
map <SOCKET, SOCKETINFO> g_clientList;

SC_MovePacket2 sc_packet;

Position g_clientPosition[2] = { {-350, -350}, {-250, -350} };
int index = 0;

int main()
{
	if (!Initialize())
		return 0;
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

int recvn(SOCKET s, char* buf, int len, int flags)
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

	Position position[2] = { {-350, -350 }, {-250, -350} };

	while (true)
	{
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

			byte playerID = g_clientList.size() - 1;
			(*iter).second.m_scPacket.m_PlayerID = playerID;
			(*iter).second.m_scPacket.m_ClientSize = g_clientList.size();
			(*iter).second.m_dataBuffer.len = sizeof(SC_MovePacket2);
			(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scPacket);
			(*iter).second.m_overlapped.hEvent = (HANDLE)(*iter).second.m_socket;

			//for (int i = 0; i < 2; ++i)
			//{
			//	(*iter).second.m_scPacket.m_Position = g_clientPosition[i];
			//	
			//	if (WSASend((*iter).second.m_socket, &(*iter).second.m_dataBuffer, 1, NULL, 0,
			//		&((*iter).second.m_overlapped), send_callback) == SOCKET_ERROR)
			//	{
			//		if (WSAGetLastError() != WSA_IO_PENDING)
			//		{
			//			printf("Error - IO pending Failure\n");
			//			closesocket(client_socket);
			//			break;
			//		}
			//	}
			//}
			for (auto iter2 = g_clientList.begin(); iter2 != g_clientList.end(); ++iter2)
			{
				(*iter).second.m_scPacket.m_Position = (*iter2).second.m_scPacket.m_Position;
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
		//PacketProcess(client_socket, g_clientList[client_socket].m_csPacket);
		PacketProcess2(client_socket, g_clientList[client_socket].m_csPacket);

		// 패킷처리가 끝났으면, 다시 recv( )를 해야함
		// 그렇지 않으면, send( )하는 동안 recv( )를 할 수 없어서 지연 됨.

		byte playerID = g_clientList.size() - 1;
		//(*iter).second.m_scPacket.m_PlayerID = playerID;
		(*iter).second.m_scPacket.m_ClientSize = g_clientList.size();
		(*iter).second.m_dataBuffer.len = sizeof(SC_MovePacket2);
		ZeroMemory(&((*iter).second.m_overlapped), sizeof(WSAOVERLAPPED));
		(*iter).second.m_overlapped.hEvent = (HANDLE)client_socket;
		// 받은 데이터를 send( )
		// send_callback을 호출
		for (int i = 0; i < 2; ++i)
		{
			(*iter).second.m_scPacket.m_Position = g_clientPosition[i];
			(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scPacket);
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
	//if (dataBytes == 0)
	//{
	//	auto iter = g_clientList.find(client_socket);
	//	if (iter != g_clientList.end())
	//	{
	//		// 해당하는 클라의 소켓을 서버에서도 지움.
	//		closesocket((*iter).second.m_socket);
	//		iter = g_clientList.erase(iter);
	//	}
	//	return;
	//}

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
		++index;

		if (WSARecv(client_socket, &(*iter).second.m_dataBuffer, 1, &receiveBytes, &flags, &((*iter).second.m_overlapped), recv_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				cout << "Error - Fail WSARecv(error_code : " << WSAGetLastError() << ")" << endl;
			// 클라가 아직 send 안함
		}
	}
}

void PacketProcess2(SOCKET& client_socket, CS_MovePacket& csPacket)
{
	auto iter = g_clientList.find(client_socket);
	if (iter != g_clientList.end())
	{
		for (auto iter2 = g_clientList.begin(); iter2 != g_clientList.end(); ++iter2)
		{
			switch (csPacket.m_Key)
			{
			case KEY_RIGHT:
				g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_X += 100;
				(*iter2).second.m_scPacket.m_Position.m_X = g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_X;
				break;
			case KEY_LEFT:
				g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_X -= 100;
				(*iter2).second.m_scPacket.m_Position.m_X -= g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_X;;
				break;
			case KEY_UP:
				g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_Y += 100;
				(*iter2).second.m_scPacket.m_Position.m_Y += g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_Y;
				break;
			case KEY_DOWN:
				g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_Y -= 100;
				(*iter2).second.m_scPacket.m_Position.m_Y -= g_clientPosition[(*iter).second.m_scPacket.m_PlayerID].m_Y;
				break;
				// KEY_IDLE
			default:
				break;
			}
		}
	}
}