#pragma once

struct SOCKETINFO
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF					m_dataBuffer;
	int								m_receiveBytes;
	int								m_sendBytes;
};

class Player;
class Network
{
public:
	Network();
	~Network();

	void err_quit(const char*);
	void err_display(const char*);
	int recvn(SOCKET, char*, int, int);

	bool Initialize();
	void Update();
	
	void InitRecvPacket();
	void RecvPacket(const map<byte, Player*>&);
	void SendPacket();

	void setCSPacket(CS_MovePacket& cs_packet)		{ m_cs_packet = cs_packet; }
	const SC_MovePacket& getSCPacket()	const { return m_sc_packet; }
	const SC_InitPacket& getSC_InitPacket()	const { return m_sc_InitPacket; }
private:
	SOCKET				m_Server_socket;

	SC_InitPacket		m_sc_InitPacket;

	SC_MovePacket	m_sc_packet;
	SC_MovePacket2	m_sc_packet2;

	CS_MovePacket	m_cs_packet;

	SOCKETINFO* m_socketInfo;
};

