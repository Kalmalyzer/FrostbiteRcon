
#include "AsynchronousServerConnection.h"

AsynchronousServerConnection::AsynchronousServerConnection(const char* host, unsigned int port, ServerRequestCallback& serverRequestCallback, ServerConnectionStateBase* stateLog, ServerConnectionTrafficBase* trafficLog)
	: AsynchronousServerConnectionBase(host, port, stateLog, trafficLog)
	, m_requestSequence(0)
	, m_serverRequestCallback(serverRequestCallback)
{
}

AsynchronousServerConnection::~AsynchronousServerConnection()
{
}

void AsynchronousServerConnection::sendRequest(Words words, ServerResponseCallback& callback)
{
	AsynchronousServerConnectionBase::sendRequest(m_requestSequence, words);
	m_outstandingRequests[m_requestSequence] = &callback;
	m_requestSequence = (m_requestSequence + 1) & BinaryRconPacket::SequenceMask;
}

void AsynchronousServerConnection::sendResponse(RequestHandle handle, const Words& words)
{
	AsynchronousServerConnectionBase::sendResponse(reinterpret_cast<uint32_t>(handle), words);
}

void AsynchronousServerConnection::onServerRequest(uint32_t sequence, Words words)
{
	m_serverRequestCallback.onServerRequest(reinterpret_cast<RequestHandle>(sequence), words);
}

void AsynchronousServerConnection::onServerResponse(uint32_t sequence, Words words)
{
	OutstandingRequests::iterator outstandingRequest = m_outstandingRequests.find(sequence);

	if (outstandingRequest == m_outstandingRequests.end())
		throw std::runtime_error("Received an unexpected response from the server");

	ServerResponseCallback* callback = outstandingRequest->second;
	m_outstandingRequests.erase(outstandingRequest);

	callback->onServerResponse(words);
}
