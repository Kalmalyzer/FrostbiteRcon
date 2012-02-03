
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ServerConnection.h"

#include "../RconLibrary/ThreadedServerConnection.h"
#include "../RconLibrary/ServerConnectionTrafficBase.h"

class TrafficLog : public ServerConnectionTrafficBase
{
public:
	TrafficLog(HWND hwnd);

	virtual void onHostLookup(const char* host) { sendMessageToConsole("Performing DNS lookup for %s...\n", host); }
	virtual void onConnecting(const char* host, unsigned int port) { sendMessageToConsole("Connecting to %s:%d...\n", host, port); }
	virtual void onConnected() { sendMessageToConsole("Connected to host.\n"); }
	virtual void onDisconnected() { sendMessageToConsole("Disconnected from host.\n"); }
	virtual void onPacketSent(const TextRconPacket& packet) { sendMessageToConsole("-> %s\n", packet.toString().c_str()); }
	virtual void onPacketReceived(const TextRconPacket& packet) { sendMessageToConsole("<- %s\n", packet.toString().c_str()); }

private:
	void sendMessageToConsole(const char* format, ...);

	HWND m_hwnd;
};

TrafficLog::TrafficLog(HWND hwnd)
	: m_hwnd(hwnd)
{
}

void TrafficLog::sendMessageToConsole(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	static char buf[BinaryRconPacket::MaxPacketSize + 1024];
	vsprintf(buf, format, args);
	va_end(args);

	SendMessage(m_hwnd, WM_USER, 0, reinterpret_cast<LPARAM>(buf));
}



class ServerRequestCallback : public ThreadedServerConnection::ServerRequestCallback
{
public:
	virtual void onServerRequest(ThreadedServerConnection::RequestHandle handle, const Words& words)
	{
	}
};

class ServerResponseCallback : public ThreadedServerConnection::ServerResponseCallback
{
public:
	virtual void onServerResponse(const Words& words)
	{
	}
};

ServerConnectionThread::ServerConnectionThread(HWND hwnd)
	: m_hwnd(hwnd)
{
}

void ServerConnectionThread::run()
{
	try
	{
		TrafficLog* trafficLog = new TrafficLog(m_hwnd);
		ServerRequestCallback* serverRequestCallback = new ServerRequestCallback();
		m_threadedServerConnection = new ThreadedServerConnection("127.0.0.1", 47203, *serverRequestCallback, trafficLog);

		while (true)
		{
			Sleep(100);
		}
	}
	catch (std::exception& e)
	{
		static char buf[1024];
		sprintf(buf, "Exception: %s", e.what());

		SendMessage(m_hwnd, WM_USER, 0, reinterpret_cast<LPARAM>(buf));
	}

}

void ServerConnectionThread::sendRequest(const Words& words)
{
	static ServerResponseCallback serverResponsecallback;
	m_threadedServerConnection->sendRequest(words, serverResponsecallback);
}