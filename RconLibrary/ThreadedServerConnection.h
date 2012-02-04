


#ifndef ThreadedServerConnection_h
#define ThreadedServerConnection_h

#include "AsynchronousServerConnectionBase.h"
#include "Mutex.h"
#include "WaitableThreadBase.h"

#include <map>
#include <queue>

class ThreadedServerConnection : public AsynchronousServerConnectionBase
{
public:
	class ServerResponseCallback
	{
	public:
		virtual void onServerResponse(const Words& words) = 0;
	};

	class ClientResponse
	{
	public:
		virtual void sendResponse(const Words& words) = 0;
	};

	class ServerRequestCallback
	{
	public:
		virtual void onServerRequest(ClientResponse& response, const Words& words) = 0;
	};

	ThreadedServerConnection(const char* host, unsigned int port, ServerRequestCallback& callback, ServerConnectionStateBase* stateLog = nullptr, ServerConnectionTrafficBase* trafficLog = nullptr);
	virtual ~ThreadedServerConnection();

	void sendRequest(Words words, ServerResponseCallback& callback);

private:
	class ClientResponseImpl : public ClientResponse
	{
	public:
		ClientResponseImpl(ThreadedServerConnection& connection, uint32_t sequence);
		virtual void sendResponse(const Words& words);
	private:
		ThreadedServerConnection& m_connection;
		uint32_t m_sequence;
	};

	void sendResponse(uint32_t sequence, const Words& words);

	template <typename T>
	class PacketQueueEntry
	{
	public:
		PacketQueueEntry(bool isResponse, uint32_t sequence, const Words& words, T requestData);

		bool m_isResponse;
		uint32_t m_sequence;
		Words m_words;
		T m_requestData;
	};

	typedef PacketQueueEntry<ServerResponseCallback*> OutgoingQueueEntry;

	typedef std::queue<OutgoingQueueEntry> OutgoingQueue;

	Mutex m_outgoingQueueMutex;
	OutgoingQueue m_outgoingQueue;



	typedef PacketQueueEntry<int> IncomingQueueEntry;

	typedef std::queue<IncomingQueueEntry> IncomingQueue;

	Mutex m_incomingQueueMutex;
	IncomingQueue m_incomingQueue;



	virtual void onServerRequest(uint32_t sequence, Words words);
	virtual void onServerResponse(uint32_t sequence, Words words);


	uint32_t m_outgoingRequestSequence;
	ServerRequestCallback& m_serverRequestCallback;

	class WorkerThread;
	friend class WorkerThread;

	class WorkerThread : public WaitableThreadBase
	{
	public:
		WorkerThread(ThreadedServerConnection& connection);

		virtual void run();

	private:
		ThreadedServerConnection& m_connection;

		typedef std::map<uint32_t, ServerResponseCallback*> OutstandingRequests;
		OutstandingRequests m_outstandingRequests;
	};

	WorkerThread* m_workerThread;
};

template <typename T>
ThreadedServerConnection::PacketQueueEntry<T>::PacketQueueEntry(bool isResponse, uint32_t sequence, const Words& words, T requestData)
	: m_isResponse(isResponse)
	, m_sequence(sequence)
	, m_words(words)
	, m_requestData(requestData)
{
}



#endif
