
#ifndef ServerConnectionTrafficBase_h
#define ServerConnectionTrafficBase_h

class TextRconPacket;

class ServerConnectionTrafficBase
{
public:
	virtual void onPacketSent(const TextRconPacket& packet) = 0;
	virtual void onPacketReceived(const TextRconPacket& packet) = 0;
};

#endif
