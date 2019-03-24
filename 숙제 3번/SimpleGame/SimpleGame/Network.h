#pragma once

struct SOCKETINFO
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF					m_dataBuffer;
	int								m_receiveBytes;
	int								m_sendBytes;
};

class Player;
class Scene;
class Network
{
public:
	Network();
	~Network();

	void err_quit(const char*);
	void err_display(const char*);
	int recvn(SOCKET, char*, int, int);

	bool Initialize();
	bool Packet_Initialize(byte);
	
	void Recv_InitPacket(Scene*);
	void Recv_UpdatePacket(Scene*, map<byte, Player*>&);
	void SendPacket();

	void setCSPacket(CS_MovePacket& cs_packet)		{ m_cs_packet = cs_packet; }
	const SC_UpdatePacket& getscUpdatePacket()	const { return m_scUpdatePacket; }

	enum PacketType { InitPacket, UpdatePacket, CS_Packet };
private:
	SOCKET				m_Server_socket;

	SC_InitPacket		m_scInitPacket;
	SC_UpdatePacket	m_scUpdatePacket;

	CS_MovePacket	m_cs_packet;

	SOCKETINFO* m_socketInfo;
};


