
#ifndef SynchronousServerConnection_h
#define SynchronousServerConnection_h

#include "RconPacket.h"

class ServerConnectionTrafficBase;
class ServerConnectionStateBase;

class SynchronousServerConnection
{
public:
	SynchronousServerConnection(const char* host, unsigned int port, ServerConnectionStateBase* stateLog = nullptr, ServerConnectionTrafficBase* trafficLog = nullptr);
	~SynchronousServerConnection();

	Words execute(const Words& request);

private:
	ServerConnectionStateBase* m_stateLog;
	ServerConnectionTrafficBase* m_trafficLog;

	// Definition lifted from WinSock2.h to avoid problems with Windows include file order
	typedef int SOCKET;

	SOCKET m_socket;
	uint32_t m_sequence;
};

#endif
