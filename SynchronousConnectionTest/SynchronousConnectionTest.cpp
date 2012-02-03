
#include "../RconLibrary/SynchronousServerConnection.h"
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

int main(void)
{
	try
	{
		StateLog stateLog;
		TrafficLog trafficLog;
		SynchronousServerConnection conn("213.163.71.95", 47201, &stateLog, &trafficLog);

		Words versionRequest = createWords("version");
		Words versionResult = conn.execute(versionRequest);
	}
	catch (const std::exception& e)
	{
		printf("Exception: %s\n", e.what());
	}
}
