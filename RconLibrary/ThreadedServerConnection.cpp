
#include "ThreadedServerConnection.h"

ThreadedServerConnection::WorkerThread::WorkerThread(ThreadedServerConnection& connection)
	: m_connection(connection)
{
}

void ThreadedServerConnection::WorkerThread::run()
{
	try
	{
		while (!isQuitRequested())
		{
			m_connection.update();

			{
				MutexScope(m_connection.m_outgoingQueueMutex);
				while (!m_connection.m_outgoingQueue.empty())
				{
					OutgoingQueueEntry entry = m_connection.m_outgoingQueue.front();
					m_connection.m_outgoingQueue.pop();

					if (!entry.m_isResponse)
					{
						m_connection.AsynchronousServerConnectionBase::sendRequest(entry.m_sequence, entry.m_words);
						m_outstandingRequests[entry.m_sequence] = entry.m_requestData;
					}
					else
					{
						m_connection.AsynchronousServerConnectionBase::sendResponse(entry.m_sequence, entry.m_words);
					}
				}
			}

			{
				MutexScope(m_connection.m_incomingQueueMutex);
				while (!m_connection.m_incomingQueue.empty())
				{
					IncomingQueueEntry entry = m_connection.m_incomingQueue.front();
					m_connection.m_incomingQueue.pop();

					if (!entry.m_isResponse)
					{
						ClientResponseImpl* clientResponse = new ClientResponseImpl(m_connection, entry.m_sequence);
						m_connection.m_serverRequestCallback.onServerRequest(*clientResponse, entry.m_words);
					}
					else
					{
						OutstandingRequests::iterator outstandingRequest = m_outstandingRequests.find(entry.m_sequence);

						if (outstandingRequest == m_outstandingRequests.end())
						{
							throw std::runtime_error("Received an unexpected response from the server");
						}
						else
						{
							ServerResponseCallback* callback = outstandingRequest->second;
							m_outstandingRequests.erase(outstandingRequest);

							callback->onServerResponse(entry.m_words);
						}
					}
				}
			}

			Sleep(100);
		}
	}
	catch (const std::exception& e)
	{
		printf("Exception: %s\n", e.what());
	}
}

ThreadedServerConnection::ClientResponseImpl::ClientResponseImpl(ThreadedServerConnection& connection, uint32_t sequence)
	: m_connection(connection)
	, m_sequence(sequence)
{
}

void ThreadedServerConnection::ClientResponseImpl::sendResponse(const Words& words)
{
	m_connection.sendResponse(m_sequence, words);
	delete this;
}

ThreadedServerConnection::ThreadedServerConnection(const char* host, unsigned int port, ServerRequestCallback& serverRequestCallback, ServerConnectionStateBase* stateLog, ServerConnectionTrafficBase* trafficLog)
	: AsynchronousServerConnectionBase(host, port, stateLog, trafficLog)
	, m_outgoingRequestSequence(0)
	, m_serverRequestCallback(serverRequestCallback)
{
	m_workerThread = new WorkerThread(*this);
}

ThreadedServerConnection::~ThreadedServerConnection()
{
	m_workerThread->requestQuit();
	delete m_workerThread;
}

void ThreadedServerConnection::sendRequest(Words words, ServerResponseCallback& callback)
{
	MutexScope mutexScope(m_outgoingQueueMutex);

	OutgoingQueueEntry entry(false, m_outgoingRequestSequence, words, &callback);
	m_outgoingQueue.push(entry);

	m_outgoingRequestSequence = (m_outgoingRequestSequence + 1) & BinaryRconPacket::SequenceMask;
}

void ThreadedServerConnection::sendResponse(uint32_t sequence, const Words& words)
{
	MutexScope mutexScope(m_outgoingQueueMutex);

	OutgoingQueueEntry entry(true, sequence, words, nullptr);
	m_outgoingQueue.push(entry);
}

void ThreadedServerConnection::onServerRequest(uint32_t sequence, Words words)
{
	MutexScope mutexScope(m_incomingQueueMutex);
	IncomingQueueEntry entry(false, sequence, words, 0);
	m_incomingQueue.push(entry);
}

void ThreadedServerConnection::onServerResponse(uint32_t sequence, Words words)
{
	MutexScope mutexScope(m_incomingQueueMutex);
	IncomingQueueEntry entry(true, sequence, words, 0);
	m_incomingQueue.push(entry);
}
