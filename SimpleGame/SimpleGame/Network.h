#pragma once

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

	void RecvPacket();
	void SendPacket();

	void setCSPacket(CS_MovePacket& cs_packet) { m_cs_packet = cs_packet; }
	const SC_MovePacket& getSCPacket()	const { return m_sc_packet; }

private:
	SOCKET				m_Server_socket;

	SC_MovePacket	m_sc_packet;
	CS_MovePacket	m_cs_packet;
};

