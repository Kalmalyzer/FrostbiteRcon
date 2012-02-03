
#ifndef ServerConnectionTrafficBase_h
#define ServerConnectionTrafficBase_h

class TextRconPacket;

class ServerConnectionTrafficBase
{
public:
	virtual void onHostLookup(const char* host) = 0;
	virtual void onConnecting(const char* host, unsigned int port) = 0;
	virtual void onConnected() = 0;
	virtual void onDisconnected() = 0;

	virtual void onPacketSent(const TextRconPacket& packet) = 0;
	virtual void onPacketReceived(const TextRconPacket& packet) = 0;
};

#endif
