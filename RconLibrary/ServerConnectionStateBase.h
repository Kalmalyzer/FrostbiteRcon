
#ifndef ServerConnectionStateBase_h
#define ServerConnectionStateBase_h

class ServerConnectionStateBase
{
public:
	virtual void onHostLookup(const char* host) = 0;
	virtual void onConnecting(const char* host, unsigned int port) = 0;
	virtual void onConnected() = 0;
	virtual void onDisconnected() = 0;
};

#endif
