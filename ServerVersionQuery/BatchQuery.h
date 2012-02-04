
#include <queue>
#include <string>
#include <vector>

#include "../RconLibrary/WaitableThreadBase.h"
#include "../RconLibrary/Mutex.h"

class SynchronousServerConnection;

class BatchQuery
{
public:
	typedef std::vector<std::string> Servers;
	typedef std::vector<std::string> Responses;
	void process(const Servers& servers, Responses& responses);

private:

	enum { MaxQueriesInFlight = 100 };

	class ServerQuery;

	class WorkerThread : public WaitableThreadBase
	{
	public:
		WorkerThread(BatchQuery& batchQuery, ServerQuery& query);

		virtual void run();

	private:
		BatchQuery& m_batchQuery;
		ServerQuery& m_query;
	};

	friend class WorkerThread;

	class ServerQuery
	{
	public:
		ServerQuery(const std::string& host, unsigned int port);

		std::string m_host;
		unsigned int m_port;
		WorkerThread* m_thread;
		std::string m_response;
	};

	typedef std::vector<ServerQuery> Queries;
	typedef std::queue<unsigned int> NotYetStarted;

	Queries m_queries;
	NotYetStarted m_notYetStarted;

	void beginQuery(unsigned int nextQuery);

	void increaseResponsesRemaining();
	void decreaseResponsesRemaining();

	Mutex m_responsesRemainingMutex;
	unsigned int m_responsesRemaining;

	void increaseQueriesInFlight();
	void decreaseQueriesInFlight();

	Mutex m_queriesInFlightMutex;
	unsigned int m_queriesInFlight;
};