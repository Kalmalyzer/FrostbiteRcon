

#ifndef AsynchronousServerConnection_h
#define AsynchronousServerConnection_h

#include "AsynchronousServerConnectionBase.h"

#include <map>

class AsynchronousServerConnection : public AsynchronousServerConnectionBase
{
public:
	class ServerResponseCallback
	{
	public:
		virtual void onServerResponse(const Words& words) = 0;
	};

	struct RequestHandleBase;
	typedef RequestHandleBase* RequestHandle;

	class ServerRequestCallback
	{
	public:
		virtual void onServerRequest(RequestHandle handle, const Words& words) = 0;
	};

	AsynchronousServerConnection(const char* host, unsigned int port, ServerRequestCallback& callback, ServerConnectionStateBase* stateLog = nullptr, ServerConnectionTrafficBase* trafficLog = nullptr);
	virtual ~AsynchronousServerConnection();

	void sendRequest(Words words, ServerResponseCallback& callback);
	void sendResponse(RequestHandle handle, const Words& words);

private:

	typedef std::map<uint32_t, ServerResponseCallback*> OutstandingRequests;

	virtual void onServerRequest(uint32_t sequence, Words words);
	virtual void onServerResponse(uint32_t sequence, Words words);

	OutstandingRequests m_outstandingRequests;

	uint32_t m_requestSequence;
	ServerRequestCallback& m_serverRequestCallback;
};

#endif
