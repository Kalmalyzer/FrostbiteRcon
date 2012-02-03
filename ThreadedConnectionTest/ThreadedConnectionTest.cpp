
#include "../RconLibrary/ThreadedServerConnection.h"
#include "../RconLibrary/ServerConnectionStateBase.h"
#include "../RconLibrary/ServerConnectionTrafficBase.h"

class StateLog : public ServerConnectionStateBase
{
public:
	virtual void onHostLookup(const char* host) { printf("Performing DNS lookup for %s...\n", host); }
	virtual void onConnecting(const char* host, unsigned int port) { printf("Connecting to %s:%d...\n", host, port); }
	virtual void onConnected() { printf("Connected to host.\n"); }
	virtual void onDisconnected() { printf("Disconnected from host.\n"); }
};

class TrafficLog : public ServerConnectionTrafficBase
{
public:
	virtual void onPacketSent(const TextRconPacket& packet) { printf("Packet sent: %s\n", packet.toString().c_str()); }
	virtual void onPacketReceived(const TextRconPacket& packet) { printf("Packet received: %s\n", packet.toString().c_str()); }
};



class ServerRequestCallback : public ThreadedServerConnection::ServerRequestCallback
{
public:
	virtual void onServerRequest(ThreadedServerConnection::ClientResponse& response, const Words& words)
	{
		printf("<- %s\n", toString(words).c_str()); 
		printf("-> OK\n");
		response.sendResponse(createWords("OK"));
	}
};

class VersionResponseCallback : public ThreadedServerConnection::ServerResponseCallback
{
public:
	virtual void onServerResponse(const Words& words)
	{
		if (words.size() == 3 && words[0] == "OK")
			printf("Server version: Game %s, build ID %s\n", words[1].c_str(), words[2].c_str());
		else
			printf("Invalid response to version query\n");
	}
};

class LoginResponseCallback : public ThreadedServerConnection::ServerResponseCallback
{
public:
	virtual void onServerResponse(const Words& words)
	{
		if (words.size() == 1)
		{
			if (words[0] == "OK")
				printf("Logged in successfully\n");
			else if (words[0] == "InvalidPassword")
				printf("Invalid password\n");
			else
				printf("Invalid response to login query\n");
		}
		else
			printf("Invalid response to login query\n");
	}
};

class ServerResponseCallback : public ThreadedServerConnection::ServerResponseCallback
{
public:
	virtual void onServerResponse(const Words& words)
	{
		printf("Server response: %s\n", toString(words).c_str());
	}
};

int main(void)
{
	try
	{
		StateLog stateLog;
		TrafficLog trafficLog;
		ServerRequestCallback serverRequestCallback;
		ThreadedServerConnection conn("213.163.71.95", 47203, serverRequestCallback, &stateLog, /* &trafficLog */ nullptr);

		printf("Sending version command\n");
		Words versionRequest = createWords("version");
		VersionResponseCallback versionCallback;
		conn.sendRequest(versionRequest, versionCallback);

		printf("Sending login command\n");
		Words loginRequest = createWords("login.plainText", "smurf");
		LoginResponseCallback loginCallback;
		conn.sendRequest(loginRequest, loginCallback);

		ServerResponseCallback serverResponseCallback;

		printf("Asking to enable events\n");
		Words enableEventsRequest = createWords("admin.eventsEnabled", "true");
		conn.sendRequest(enableEventsRequest, serverResponseCallback);

		Sleep(2000);

		printf("Sending end-round request\n");
		Words endRoundRequest = createWords("mapList.restartRound");
		conn.sendRequest(endRoundRequest, serverResponseCallback);

		while (true)
		{
			Sleep(1000);
		}
	}
	catch (const std::exception& e)
	{
		printf("Exception: %s\n", e.what());
	}
}
