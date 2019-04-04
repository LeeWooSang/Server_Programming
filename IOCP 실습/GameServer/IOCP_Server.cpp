#include <iostream>
#include "Protocol.h"
#include <thread>
#include <vector>
using namespace std;
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER	1024
// 플레이어 시작 위치
#define START_X			4
#define START_Y			4

// 확장 오버렙트 구조체
struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF					dataBuffer;
	char							messageBuffer[MAX_BUFFER];
	bool							is_recv;
	bool							is_send;
};

// 클라이언트 정보 (I/O를 하기위해 필요한 정보)
struct SOCKETINFO
{
	bool			connected;
	OVER_EX	over;
	SOCKET	socket;
	char			packet_buf[MAX_BUFFER];
	int				prev_size;
	// 플레이어 정보가 들어감.
	int				sendBytes;

	// 접속한 플레이어의 x, y 좌표
	int x, y;
};

// 클라를 키로 찾음
SOCKETINFO clients[MAX_USER];
// IOCP 객체 핸들
HANDLE g_IOCP;

void Initialize();
void Worker_Thread();
char Get_New_ID();
void Do_Accept();
void Do_Recv(char id);
void Process_Packet(char id, char* buf);

void 	Send_Login_Ok_Packet(char to);
void 	Send_Put_Player_Packet(char to, char obj);
void 	Send_Pos_Packet(char to, char obj);
void Send_Remove_Player_Packet(char to, char id);

void Send_Packet(char key, char* packet);
void Disconnect(int id);
// 에러출력
void Error_Display(const char* msg, int err_no);

int main()
{
	// 워커 스레드 생성
	vector<thread> worker_threads;

	_wsetlocale(LC_ALL, L"korean");
	// 에러발생시 한글로 출력되도록 명령
	wcout.imbue(locale("korean"));

	Initialize();

	// IOCP 객체 생성
	g_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	for (int i = 0; i < 4; ++i)
	{
		worker_threads.emplace_back(thread{ Worker_Thread });
	}

	// accept 스레드 생성
	thread accept_thread{ Do_Accept };

	// 스레드 종료 대기
	for (auto& threads : worker_threads)
		threads.join();

	accept_thread.join();

	CloseHandle(g_IOCP);
}

void Initialize()
{
	for (auto& client : clients)
	{
		client.connected = false;
	}
}

void Worker_Thread()
{
	while (true)
	{
		DWORD io_byte;
		unsigned long key;
		OVER_EX* lpover_ex;
		// lpover에 recv인지 send인지 정보를 넣어야 됨.
		BOOL is_error = GetQueuedCompletionStatus(g_IOCP, &io_byte, &key, reinterpret_cast<LPWSAOVERLAPPED*>(&lpover_ex), INFINITE);
		
		//  GetQueuedCompletionStatus( )가 에러인지 아닌지 확인한다
		if (is_error == FALSE)
			Error_Display(" GetQueuedCompletionStatus()", WSAGetLastError());

		// 클라와 끊어졌다면 (클라가 나갔을 때)
		if (io_byte == 0)
		{
			Disconnect(key);
		}

		// recv와 send가 끝났을 때 처리
		if (lpover_ex->is_recv)
		{
			// 남은 패킷 크기
			int rest_size = io_byte;
			char* p = lpover_ex->messageBuffer;
			char packet_size = 0;

			// 남은 패킷이 있을 경우
			if (clients[key].prev_size > 0)
				packet_size = clients[key].packet_buf[0];

			while (rest_size > 0)
			{
				if (packet_size == 0)
					packet_size = p[0];

				int required = packet_size - clients[key].prev_size;
				// 패킷을 만들 수 있다면,
				if (rest_size >= required)
				{
					memcpy(clients[key].packet_buf + clients[key].prev_size, p, required);
					Process_Packet(key, clients[key].packet_buf);
					rest_size -= required;
					p += required;
					packet_size = 0;
				}
				// 패킷을 만들 수 없다면,
				else
				{
					memcpy(clients[key].packet_buf + clients[key].prev_size, p, rest_size);
					rest_size = 0;
				}
			}
			Do_Recv(key);
		}
		else
		{
			// send일 때,
			delete lpover_ex;
		}

	}
}

char Get_New_ID()
{
	// 아이디가 있을 때까지, 루프돌면서 기다리게함
	while (true)
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (clients[i].connected == false)
			{
				clients[i].connected = true;
				return i;
			}
		}
	}
}

void Do_Accept()
{
	// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return;
	}

	// 1. 소켓생성 
	// Overlapped I/O를 하기 위해서 WSASocket 마지막 인자 값에 WSA_FLAG_OVERLAPPED를 넣어줌
	// WSASocketA,W 오류나면, 멀티바이트 -> 유니코드로 변경

	SOCKET listenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Error - Invalid socket\n";
		return;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	// INADDR_ANY : 어떤 주소든 받겠다.
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. 소켓설정
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		cout << "Error - Fail bind\n";
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return;
	}

	// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "Error - Fail listen\n";
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (true)
	{
		clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Error - Accept Failure\n";
			return;
		}
		char new_id = Get_New_ID();

		memset(&clients[new_id], 0x00, sizeof(struct SOCKETINFO));
		clients[new_id].socket = clientSocket;
		clients[new_id].over.dataBuffer.len = MAX_BUFFER;
		clients[new_id].over.dataBuffer.buf = clients[clientSocket].over.messageBuffer;
		clients[new_id].over.is_recv = true;
		clients[new_id].x = START_X;
		clients[new_id].y = START_Y;
		flags = 0;

		// Recv하기전에 IOCP에 연결해줌
		// 클라 소켓을 핸들로 형변환
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_IOCP, new_id, 0);

		clients[new_id].connected = true;

		// 너가 누구인지 보내주어야함
		Send_Login_Ok_Packet(new_id);
		
		// 접속된 다른 유저에게도 보내야 됨
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (clients[i].connected == false) continue;

			Send_Put_Player_Packet(i, new_id);
		}

		for (int i = 0; i < MAX_USER; ++i)
		{
			// 나에게 다른 애들 정보도 보내주어야 함
			if (clients[i].connected == false) continue;
			
				// 내정보는 안보내도됨
			if (new_id == i) continue;
				
			Send_Put_Player_Packet(new_id, i);
		}
		Do_Recv(new_id);
	}

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();
}

void Do_Recv(char id)
{
	DWORD flags = 0;
	SOCKET client_s = clients[id].socket;
	OVER_EX* over = &clients[id].over;

	// WSASend(응답에 대한)의 콜백일 경우
	over->dataBuffer.len = MAX_BUFFER;
	over->dataBuffer.buf = over->messageBuffer;

	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));

	if (WSARecv(client_s, &over->dataBuffer, 1, nullptr, &flags, &(over->overlapped), nullptr) == SOCKET_ERROR)
	{
		int error_no = WSAGetLastError();
		if (error_no != WSA_IO_PENDING)
		{
			Error_Display("WSARecv() Error - ", error_no);
		}
	}

}

// 컨텐츠
void Process_Packet(char id, char* buf)
{
	CS_Packet_Up* packet = reinterpret_cast<CS_Packet_Up*>(buf);
	char x = clients[id].x;
	char y = clients[id].y;

	switch (packet->type)
	{
	case CS_UP:
		--y;
		if (y < 0)
			y = 0;
		break;

	case CS_DOWN:
		++y;
		if (y >= WORLD_HEIGHT)
			y = WORLD_HEIGHT - 1;
		break;

	case CS_LEFT:
		if (x > 0)
			x--;
		break;

	case CS_RIGHT:
		if (x < WORLD_WIDTH - 1)
			x++;
		break;

	default:
		cout << "Unknown Packet Type" << endl;
		while (true);
	}
	clients[id].x = x;
	clients[id].y = y;

	// 접속한 유저한테 위치를 보내줌
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i].connected == true)
			Send_Pos_Packet(i, id);
	}
}

void 	Send_Login_Ok_Packet(char to)
{
	SC_Packet_Login_OK packet;
	packet.id = to;
	packet.size = sizeof(SC_Packet_Login_OK);
	packet.type = SC_LOGIN_OK;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void 	Send_Put_Player_Packet(char to, char obj)
{
	SC_Packet_Put_Player packet;
	packet.id = obj;
	packet.size = sizeof(SC_Packet_Put_Player);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[obj].x;
	packet.y = clients[obj].y;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

// 누구에게, 오브젝트를
void 	Send_Pos_Packet(char to, char obj)
{
	SC_Packet_Pos packet;
	packet.id = obj;
	packet.size = sizeof(SC_Packet_Pos);
	packet.type = SC_POS;
	packet.x = clients[obj].x;
	packet.y = clients[obj].y;

	Send_Packet(to, reinterpret_cast<char*>(&packet));

}
void Send_Remove_Player_Packet(char to, char id)
{
	SC_Packet_Remove_Player packet;
	packet.id = to;
	packet.size = sizeof(SC_Packet_Remove_Player);
	packet.type = SC_REMOVE_PLAYER;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void Send_Packet(char key, char* packet)
{
	SOCKET client_s = clients[key].socket;
	OVER_EX* over = new OVER_EX;

	over->dataBuffer.len = packet[0];
	over->dataBuffer.buf = over->messageBuffer;
	// 패킷의 내용을 버퍼에 복사
	memcpy(over->messageBuffer, packet, packet[0]);

	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	over->is_recv = false;

	if (WSASend(client_s, &over->dataBuffer, 1, nullptr, 0, &(over->overlapped), nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - Fail WSARecv(error_code : " << WSAGetLastError() << endl;
		}
	}
}

void Disconnect(int id)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if(clients[i].connected == true)
			Send_Remove_Player_Packet(i, id);
	}
	closesocket(clients[id].socket);
	clients[id].connected = false;
}

void Error_Display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	cout << msg;
	wcout << L"설명 : " << lpMsgBuf << endl;

	// 에러발생시 무한루프로 멈추게함
	while (true);

	LocalFree(lpMsgBuf);
}