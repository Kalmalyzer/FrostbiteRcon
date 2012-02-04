
#ifndef ServerConnection_h
#define ServerConnection_h

#include "../RconLibrary/WaitableThreadBase.h"
#include "../RconLibrary/ThreadedServerConnection.h"
#include "../RconLibrary/RconPacket.h"

#include <Windows.h>

class ServerConnectionThread : public WaitableThreadBase
{
public:
	ServerConnectionThread(HWND hwnd);

	virtual void run();

	void sendRequest(const Words& words);

private:
	HWND m_hwnd;

	ThreadedServerConnection* m_threadedServerConnection;
};

#endif
