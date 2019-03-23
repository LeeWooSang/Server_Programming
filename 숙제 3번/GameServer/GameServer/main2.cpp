#include "Defines.h"
#include "Player.h"

struct SOCKETINFO
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF					m_dataBuffer;

	SOCKET m_socket;

	SC_InitPacket		m_scInitPacket;
	SC_UpdatePacket	m_scUpdatePacket;
	CS_MovePacket	m_csPacket;

	byte			m_ID;
	Position	m_Position;

	int								m_receiveBytes;
	int								m_sendBytes;
};

void err_quit(const char*);
void err_display(const char*);
bool Initialize();
void PacketProcess(SOCKET& client_socket, CS_MovePacket& csPacket);

// �ݹ��Լ� ����
void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

// �ڡڡڡڡڼ������� � Ŭ���̾�Ʈ���� �Դ��� �� �� �ְ� �ؾ���
map <SOCKET, SOCKETINFO> g_clientList;

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

bool Initialize()
{
	int retval = 0;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	// Socket( )
	// ��� ���� ����
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

	// namespace std�� �ִ� bind �Լ��� Winsock2�� �ִ� bind �Լ��� ���ļ� ����
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
	cout << "��� ���� ���� �Ϸ�" << endl;

	SOCKET client_socket;
	SOCKADDR_IN client_addr;
	int addrlen = sizeof(client_addr);
	ZeroMemory(&client_addr, addrlen);
	DWORD flags = 0;

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
		// accept( )
		client_socket = accept(listen_socket, (SOCKADDR*)&client_addr, &addrlen);
		if (client_socket == INVALID_SOCKET)
		{
			err_display("accept( )");
			break;
		}
		cout << "\n[ TCP ���� ] Ŭ���̾�Ʈ ���� - IP : " << inet_ntoa(client_addr.sin_addr)
			<< ", ��Ʈ ��ȣ : " << ntohs(client_addr.sin_port) << endl;

		// Ŭ���̾�Ʈ ������ 0���� �ʱ�ȭ
		g_clientList.emplace(client_socket, SOCKETINFO{});
		auto iter = g_clientList.find(client_socket);
		if (iter != g_clientList.end())
		{
			// Ŭ���̾�Ʈ ������ 0���� �ʱ�ȭ
			ZeroMemory(&((*iter).second), sizeof(SOCKETINFO));
			(*iter).second.m_socket = client_socket;

			(*iter).second.m_ID = g_clientList.size() - 1;
			(*iter).second.m_Position = position[(*iter).second.m_ID];

			(*iter).second.m_scInitPacket.m_ClientSize = g_clientList.size();
			(*iter).second.m_dataBuffer.len = sizeof(SC_InitPacket);
			(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scInitPacket);
			(*iter).second.m_overlapped.hEvent = (HANDLE)(*iter).second.m_socket;

			byte count = g_clientList.size() - 1;
			for (auto iter2 = g_clientList.begin(); iter2 != g_clientList.end(); ++iter2)
			{
				(*iter).second.m_scInitPacket.m_PlayerID = (*iter2).second.m_ID;
				(*iter).second.m_scInitPacket.m_Position = (*iter2).second.m_Position;
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


// ��Ŷó���� recv_callback���� ����Ѵ�.
// recv_callback 1���� �Լ����� ���ϸ��� �ٸ��� ó���Ѵ�.
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	// �ε����� �־� ��. 
	// ���ٸ� ����� �� �� ����
	SOCKET client_socket = reinterpret_cast<int>(overlapped->hEvent);
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	// Ŭ���̾�Ʈ���� closesocket( )�� ȣ���� ���
	if (dataBytes == 0)
	{
		auto iter = g_clientList.find(client_socket);
		if (iter != g_clientList.end())
		{
			// �ش��ϴ� Ŭ���� ������ ���������� ����.
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

		// ��Ŷó���� ��������, �ٽ� recv( )�� �ؾ���
		// �׷��� ������, send( )�ϴ� ���� recv( )�� �� �� ��� ���� ��.

		byte playerID = g_clientList.size() - 1;
		(*iter).second.m_scUpdatePacket.m_ClientSize = g_clientList.size();
		(*iter).second.m_dataBuffer.len = sizeof(SC_UpdatePacket);
		ZeroMemory(&((*iter).second.m_overlapped), sizeof(WSAOVERLAPPED));
		(*iter).second.m_overlapped.hEvent = (HANDLE)client_socket;
		(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scUpdatePacket);

		byte count = g_clientList.size() - 1;
		for (auto iter2 = g_clientList.begin(); iter2 != g_clientList.end(); ++iter2)
		{
			(*iter).second.m_scUpdatePacket.m_PlayerID = (*iter2).second.m_ID;
			(*iter).second.m_scUpdatePacket.m_Position = (*iter2).second.m_Position;

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

	// Ŭ���̾�Ʈ���� closesocket( )�� ȣ���� ���
	//if (dataBytes == 0)
	//{
	//	auto iter = g_clientList.find(client_socket);
	//	if (iter != g_clientList.end())
	//	{
	//		// �ش��ϴ� Ŭ���� ������ ���������� ����.
	//		closesocket((*iter).second.m_socket);
	//		iter = g_clientList.erase(iter);
	//	}
	//	return;
	//}

	auto iter = g_clientList.find(client_socket);
	if (iter != g_clientList.end())
	{
		//cout << "TRACE - Send : (" << dataBytes << " bytes)" << endl;

		// WSASend(���信 ����)�� �ݹ��� ���
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
			// Ŭ�� ���� send ����
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
			(*iter).second.m_Position.m_X += 100;
			break;
		case KEY_LEFT:
			(*iter).second.m_Position.m_X -= 100;
			break;
		case KEY_UP:
			(*iter).second.m_Position.m_Y += 100;
			break;
		case KEY_DOWN:
			(*iter).second.m_Position.m_Y -= 100;
			break;
			// KEY_IDLE
		default:
			break;
		}
	}

}