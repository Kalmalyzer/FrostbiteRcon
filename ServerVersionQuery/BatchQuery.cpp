
#include "BatchQuery.h"

#include "../RconLibrary/SynchronousServerConnection.h"

#include <cstring>


BatchQuery::WorkerThread::WorkerThread(BatchQuery& batchQuery, ServerQuery& query)
	: m_batchQuery(batchQuery)
	, m_query(query)
{
	m_batchQuery.increaseQueriesInFlight();
}

void BatchQuery::WorkerThread::run()
{
	try
	{
		SynchronousServerConnection connection(m_query.m_host.c_str(), m_query.m_port, nullptr);

		Words versionRequest = createWords("version");
		Words versionResult = connection.execute(versionRequest);

		std::string response;
		for (Words::const_iterator word = versionResult.begin(), end = versionResult.end(); word != end; ++word)
		{
			if (word != versionResult.begin())
				response += " ";

			response += *word;
		}

		m_query.m_response = response;
	}
	catch (std::exception& e)
	{
		m_query.m_response = e.what();
	}

	m_batchQuery.decreaseQueriesInFlight();
	m_batchQuery.decreaseResponsesRemaining();
}

BatchQuery::ServerQuery::ServerQuery(const std::string& host, unsigned int port)
	: m_host(host)
	, m_port(port)
	, m_thread(nullptr)
{
}

void BatchQuery::increaseResponsesRemaining()
{
	MutexScope scope(m_responsesRemainingMutex);
	m_responsesRemaining++;
}

void BatchQuery::decreaseResponsesRemaining()
{
	MutexScope scope(m_responsesRemainingMutex);
	m_responsesRemaining--;
}

void BatchQuery::increaseQueriesInFlight()
{
	MutexScope scope(m_queriesInFlightMutex);
	m_queriesInFlight++;
}

void BatchQuery::decreaseQueriesInFlight()
{
	MutexScope scope(m_queriesInFlightMutex);
	m_queriesInFlight--;
}

void BatchQuery::beginQuery(unsigned int nextQuery)
{
	ServerQuery& query = m_queries[nextQuery];
	query.m_thread = new WorkerThread(*this, query);
}

void BatchQuery::process(const Servers& servers, Responses& responses)
{
	unsigned int numServers = servers.size();

	for (unsigned int i = 0; i < numServers; ++i)
	{
		const std::string& server = servers[i];
		size_t separatorPos = server.find(':');
		std::string host = server.substr(0, separatorPos);
		unsigned int port = atoi(server.substr(separatorPos + 1).c_str());
		ServerQuery query(host, port);
		m_queries.push_back(query);
	}

	for (unsigned int i = 0; i < numServers; ++i)
		m_notYetStarted.push(i);

	m_responsesRemaining = numServers;
	m_queriesInFlight = 0;

	while (true)
	{
		{
			MutexScope scope(m_responsesRemainingMutex);

			if (!m_responsesRemaining)
				break;
		}


		while (true)
		{
			{
				MutexScope scope(m_queriesInFlightMutex);

				if (m_queriesInFlight >= MaxQueriesInFlight || m_notYetStarted.empty())
					break;
			}

			unsigned int nextQuery = m_notYetStarted.front();
			m_notYetStarted.pop();

			beginQuery(nextQuery);
		}

		Sleep(100);
	}

	for (unsigned int i = 0; i < numServers; ++i)
		responses.push_back(m_queries[i].m_response);
}
