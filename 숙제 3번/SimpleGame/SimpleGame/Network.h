#pragma once

struct SOCKETINFO
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF					m_dataBuffer;
	int								m_receiveBytes;
	int								m_sendBytes;
};

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

	void setCSPacket(CS_MovePacket& cs_packet) 
	{ 
		m_cs_packet = cs_packet; 

		if (m_socketInfo)
		{
			m_socketInfo->m_dataBuffer.len = sizeof(m_cs_packet);
			m_socketInfo->m_dataBuffer.buf = reinterpret_cast<char*>(&m_cs_packet);

			//if(reinterpret_cast<CS_MovePacket*>(m_socketInfo->m_dataBuffer.buf)->m_Key == KEY_DOWN)
		}
	}
	const SC_MovePacket& getSCPacket()	const { return m_sc_packet; }

private:
	SOCKET				m_Server_socket;

	SC_MovePacket	m_sc_packet;
	CS_MovePacket	m_cs_packet;

	SOCKETINFO* m_socketInfo;
};

