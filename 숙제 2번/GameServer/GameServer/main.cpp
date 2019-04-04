#include "Defines.h"

void err_quit(const char*);
void err_display(const char*);
int recvn(SOCKET, char*, int, int);
bool Initialize();
void Update(SOCKET& client_socket);
void KeyDistribute(CS_MovePacket&);

SC_MovePacket sc_packet;
int main()
{
	sc_packet.m_X = -50.f;
	sc_packet.m_Y = -350.f;

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
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);

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
	server_addr.sin_port = htons(PORT);
	
	// namespace std에 있는 bind 함수와 Winsock2에 있는 bind 함수와 겹쳐서 구분
	retval = ::bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (retval == SOCKET_ERROR)
	{
		err_quit("bind( )");
		return false;
	}

	// listen( )
	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		err_quit("listen( )");
		return false;
	}
	cout << "대기 소켓 생성 완료" << endl;

	SOCKET client_socket;
	SOCKADDR_IN client_addr;
	int addrlen = 0;

	while (true)
	{
		// accept( )
		addrlen = sizeof(client_addr);
		client_socket = accept(listen_socket, (SOCKADDR*)&client_addr, &addrlen);
		if (client_socket == INVALID_SOCKET)
		{
			err_display("accept( )");
			break;
		}
		cout << "\n[ TCP 서버 ] 클라이언트 접속 - IP : " << inet_ntoa(client_addr.sin_addr)
			<< ", 포트 번호 : " << ntohs(client_addr.sin_port) << endl;

		Update(client_socket);
		break;
	}

	closesocket(listen_socket);

	WSACleanup();

	return true;
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
		// 데이터 받기
		retval = recvn(client_socket, (char*)&cs_packet, sizeof(cs_packet), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recvn( )");
			break;
		}
		// 받은 데이터
		KeyDistribute(cs_packet);
	}
	closesocket(client_socket);
}

void KeyDistribute(CS_MovePacket& key)
{
	switch (key.m_Key)
	{
	case KEY_RIGHT: 
		sc_packet.m_X += 100;
		break;
	case KEY_LEFT:
		sc_packet.m_X -= 100;
		break;
	case KEY_UP:
		sc_packet.m_Y += 100;
		break;
	case KEY_DOWN:
		sc_packet.m_Y -= 100;
		break;
		// KEY_IDLE
	default:
		break;
	}
}