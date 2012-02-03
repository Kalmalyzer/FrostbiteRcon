
#include "../RconLibrary/ThreadedServerConnection.h"
#include "../RconLibrary/ServerConnectionTrafficBase.h"

class TrafficLog : public ServerConnectionTrafficBase
{
public:
	virtual void onHostLookup(const char* host) { printf("Performing DNS lookup for %s...\n", host); }
	virtual void onConnecting(const char* host, unsigned int port) { printf("Connecting to %s:%d...\n", host, port); }
	virtual void onConnected() { printf("Connected to host.\n"); }
	virtual void onDisconnected() { printf("Disconnected from host.\n"); }
	virtual void onPacketSent(const TextRconPacket& packet) { printf("Packet sent: %s\n", packet.toString().c_str()); }
	virtual void onPacketReceived(const TextRconPacket& packet) { printf("Packet received: %s\n", packet.toString().c_str()); }
};



bool done = false;

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
		//done = true;
	}
};

int main(void)
{
	try
	{
		TrafficLog trafficLog;
		ServerRequestCallback serverRequestCallback;
		ThreadedServerConnection conn("213.163.71.95", 47203, serverRequestCallback, &trafficLog);

		Words versionRequest = createWords("version");
		ServerResponseCallback versionCallback;
		conn.sendRequest(versionRequest, versionCallback);

		Words loginRequest = createWords("login.plainText", "smurfsmurf3");
		ServerResponseCallback loginCallback;
		conn.sendRequest(loginRequest, loginCallback);

		Words enableEventsRequest = createWords("admin.eventsEnabled", "true");
		ServerResponseCallback enableEventsCallback;
		conn.sendRequest(enableEventsRequest, enableEventsCallback);

		while (!done)
		{
			Sleep(100);
		}
	}
	catch (const std::exception& e)
	{
		printf("Exception: %s\n", e.what());
	}
}
