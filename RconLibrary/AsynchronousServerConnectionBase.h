
#ifndef AsynchronousServerConnectionBase_h
#define AsynchronousServerConnectionBase_h

#include <string>
#include <vector>
#include <cstdint>

#include "RconPacket.h"
#include "NonCopyable.h"

class ServerConnectionStateBase;
class ServerConnectionTrafficBase;

class AsynchronousServerConnectionBase : NonCopyable
{
public:
	AsynchronousServerConnectionBase(const char* host, unsigned int port, ServerConnectionStateBase* stateLog = nullptr, ServerConnectionTrafficBase* trafficLog = nullptr);
	virtual ~AsynchronousServerConnectionBase();

	void update();

	virtual void onServerRequest(uint32_t sequence, Words words) = 0;
	virtual void onServerResponse(uint32_t sequence, Words words) = 0;
 
protected:
	void sendRequest(uint32_t sequence, Words words);
	void sendResponse(uint32_t sequence, Words words);

private:
	ServerConnectionStateBase* m_stateLog;
	ServerConnectionTrafficBase* m_trafficLog;

	uint8_t m_receiveBuffer[BinaryRconPacket::MaxPacketSize];
	unsigned int m_receivedPacketBytes;

	// Definition lifted from WinSock2.h to avoid problems with Windows include file order
	typedef int SOCKET;

	SOCKET m_socket;
};

#endif
