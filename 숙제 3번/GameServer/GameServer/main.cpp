#include "Defines.h"

struct SOCKETINFO
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF					m_dataBuffer;

	SOCKET m_socket;

	char m_messageBuffer[BUFSIZE];

	SC_MovePacket m_scPacket;
	CS_MovePacket m_csPacket;

	int								m_receiveBytes;
	int								m_sendBytes;
};

void err_quit(const char*);
void err_display(const char*);
int recvn(SOCKET, char*, int, int);
bool Initialize();
void Update(SOCKET&);
void Update(SOCKET&, DWORD&);
void KeyDistribute(SOCKET&, CS_MovePacket&);

// �ݹ��Լ� ����
void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

// �ڡڡڡڡڼ������� � Ŭ���̾�Ʈ���� �Դ��� �� �� �ְ� �ؾ���
map <SOCKET, SOCKETINFO> g_clientList;

SC_MovePacket sc_packet;
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
	// ��� ���� ����
	//SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
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
	static float x = -350.f, y = -350.f;

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

			(*iter).second.m_scPacket.m_PlayerID = g_clientList.size() - 1;
			(*iter).second.m_scPacket.m_ClientSize = g_clientList.size();

			(*iter).second.m_scPacket.m_Position[(*iter).second.m_scPacket.m_PlayerID].m_X = x;
			(*iter).second.m_scPacket.m_Position[(*iter).second.m_scPacket.m_PlayerID].m_Y = y;

			//(*iter).second.m_dataBuffer.len = BUFSIZE;
			//(*iter).second.m_dataBuffer.buf = (*iter).second.m_messageBuffer;
			(*iter).second.m_dataBuffer.len = sizeof(SC_MovePacket);
			(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scPacket);
			// ��ø ��Ĺ�� �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ��ش�.
			(*iter).second.m_overlapped.hEvent = (HANDLE)(*iter).second.m_socket;

			x += 100;
		}
		else
		{
			closesocket(client_socket);
			break;
		}

		Update(client_socket, flags);
		//break;
	}

	closesocket(listen_socket);

	WSACleanup();

	return true;
}

void Update(SOCKET& client_socket, DWORD& flag)
{
		// 1 : ���� ī��Ʈ
		// flag�� NULL�� ������ �ȵ� => 0���� �ƴ��� Ȯ���ϱ� ������
		// Ŭ���̾�Ʈ���� �����ִ� overlapped ����ü�� �־���
		// recv_callback���� ��Ŷ�� ó����.
		auto iter = g_clientList.find(client_socket);
		if (iter != g_clientList.end())
		{
			if (WSASend((*iter).second.m_socket, &(*iter).second.m_dataBuffer, 1, nullptr, 0, 
				&((*iter).second.m_overlapped), send_callback) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("Error - IO pending Failure\n");
					return;
				}
			}
			//// ����� I/O�� �߻��� ���
			//else
			//{
			//	cout << "Non Overlapped Send return" << endl;
			//	return;
			//}
		}
		else
			return;
}

void Update(SOCKET& client_socket)
{
	while (true)
	{
		int retval = send(client_socket, (char*)&sc_packet, sizeof(sc_packet), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send( )");
			break;
		}

		CS_MovePacket cs_packet;
		// ������ �ޱ�
		retval = recvn(client_socket, (char*)&cs_packet, sizeof(cs_packet), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recvn( )");
			break;
		}
		// ���� ������
		KeyDistribute(client_socket, cs_packet);
	}
	closesocket(client_socket);
}

void KeyDistribute(SOCKET& client_socket, CS_MovePacket& key)
{
	auto iter = g_clientList.find(client_socket);
	if (iter != g_clientList.end())
	{
		switch (key.m_Key)
		{
		case KEY_RIGHT:
			(*iter).second.m_scPacket.m_Position[(*iter).second.m_scPacket.m_PlayerID].m_X += 100;
			break;
		case KEY_LEFT:
			(*iter).second.m_scPacket.m_Position[(*iter).second.m_scPacket.m_PlayerID].m_X -= 100;
			break;
		case KEY_UP:
			(*iter).second.m_scPacket.m_Position[(*iter).second.m_scPacket.m_PlayerID].m_Y += 100;
			break;
		case KEY_DOWN:
			(*iter).second.m_scPacket.m_Position[(*iter).second.m_scPacket.m_PlayerID].m_Y -= 100;
			break;
			// KEY_IDLE
		default:
			break;
		}
	}
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
		KeyDistribute(client_socket, g_clientList[client_socket].m_csPacket);
		// ��Ŷó���� ��������, �ٽ� recv( )�� �ؾ���
		// �׷��� ������, send( )�ϴ� ���� recv( )�� �� �� ��� ���� ��.

		(*iter).second.m_dataBuffer.len = sizeof(SC_MovePacket);
		(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_scPacket);
		ZeroMemory(&((*iter).second.m_overlapped), sizeof(WSAOVERLAPPED));
		(*iter).second.m_overlapped.hEvent = (HANDLE)client_socket;
		// ���� �����͸� send( )
		// send_callback�� ȣ��
		if (WSASend(client_socket, &(*iter).second.m_dataBuffer, 1, &dataBytes, 0, &((*iter).second.m_overlapped), send_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				cout << "Error - Fail WSASend(error_code : " << WSAGetLastError() << ")" << endl;
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
		//cout << "TRACE - Send : (" << dataBytes << " bytes)" << endl;

		// WSASend(���信 ����)�� �ݹ��� ���
		//(*iter).second.m_dataBuffer.len = BUFSIZE;
		//(*iter).second.m_dataBuffer.buf = (*iter).second.m_messageBuffer;
		(*iter).second.m_dataBuffer.len = sizeof(CS_MovePacket);
		(*iter).second.m_dataBuffer.buf = reinterpret_cast<char*>(&(*iter).second.m_csPacket);
		ZeroMemory(&((*iter).second.m_overlapped), sizeof(WSAOVERLAPPED));
		(*iter).second.m_overlapped.hEvent = (HANDLE)client_socket;

		// Ŭ���̾�Ʈ���� ���� �����͸� �ٽ� ����
		if (WSARecv(client_socket, &(*iter).second.m_dataBuffer, 1, &receiveBytes, &flags, &((*iter).second.m_overlapped), recv_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				cout << "Error - Fail WSARecv(error_code : " << WSAGetLastError() << ")" << endl;
		}
	}
}
