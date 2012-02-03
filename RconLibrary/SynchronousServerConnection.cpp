
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "SynchronousServerConnection.h"
#include "ServerConnectionStateBase.h"
#include "ServerConnectionTrafficBase.h"

#include <cstring>
#include <WinSock2.h>

SynchronousServerConnection::SynchronousServerConnection(const char* host, unsigned int port, ServerConnectionStateBase* stateLog, ServerConnectionTrafficBase* trafficLog)
	: m_stateLog(stateLog)
	, m_trafficLog(trafficLog)
	, m_sequence(0)
{
	WORD versionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
	int error = WSAStartup(versionRequested, &wsaData);

	if (error)
		throw std::runtime_error("Unable to initialize Winsock");

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0)
	{
		WSACleanup();
		throw std::runtime_error("Too old Winsock version");
	}

	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_stateLog)
		m_stateLog->onHostLookup(host);

	const hostent* hostEntry = gethostbyname(host);
	if (!hostEntry)
	{
		WSACleanup();
		throw std::runtime_error("Unable to translate hostname to IP address");
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = (reinterpret_cast<const in_addr*>(hostEntry->h_addr))->s_addr;
	sin.sin_port = htons(port);

	if (m_stateLog)
		m_stateLog->onConnecting(host, port);

	if (connect(m_socket, reinterpret_cast<const sockaddr*>(&sin), sizeof sin) != 0)
	{
		WSACleanup();
		throw std::runtime_error("Error while connecting to remote host");
	}

	if (m_stateLog)
		m_stateLog->onConnected();
}

SynchronousServerConnection::~SynchronousServerConnection()
{
	closesocket(m_socket);

	if (m_stateLog)
		m_stateLog->onDisconnected();

	WSACleanup();
}

Words SynchronousServerConnection::execute(const Words& request)
{
	TextRconPacket textRequest(true, false, m_sequence, request);
	m_sequence = (m_sequence + 1) & BinaryRconPacket::SequenceMask;

	BinaryRconPacket binaryRequest(textRequest);
	const uint8_t* requestData;
	unsigned int requestLength;
	binaryRequest.getBuffer(requestData, requestLength);

	int bytesSent = send(m_socket, reinterpret_cast<const char*>(requestData), requestLength, 0);

	if (bytesSent != requestLength)
		throw std::runtime_error("Error when sending request");

	if (m_trafficLog)
		m_trafficLog->onPacketSent(textRequest);

	Words response;

	std::vector<uint8_t> receivedBytes;

	uint8_t responseHeaderBuf[BinaryRconPacketHeader::Size];
	for (unsigned int receivedBytes = 0; receivedBytes < BinaryRconPacketHeader::Size; )
	{
		int recvSize = recv(m_socket, reinterpret_cast<char*>(responseHeaderBuf + receivedBytes), (sizeof responseHeaderBuf) - receivedBytes, 0);
		if (recvSize == SOCKET_ERROR)
			throw std::runtime_error("Error while receiving response");
		else if (!recvSize)
			throw std::runtime_error("The socket has been closed");
		receivedBytes += recvSize;
	}

	BinaryRconPacketHeader binaryRconPacketHeader(responseHeaderBuf);

	if (!binaryRconPacketHeader.isValid())
		throw std::runtime_error("Received an invalid packet");

	uint32_t binaryRconResponsePacketSize = binaryRconPacketHeader.getPacketSize();
	uint32_t binaryRconResponseBodySize = binaryRconResponsePacketSize - BinaryRconPacketHeader::Size;

	uint8_t* responseBodyBuf = new uint8_t[binaryRconResponseBodySize];

	for (unsigned int receivedBytes = 0; receivedBytes < binaryRconResponseBodySize; )
	{
		int recvSize = recv(m_socket, reinterpret_cast<char*>(responseBodyBuf + receivedBytes), binaryRconResponseBodySize - receivedBytes, 0);
		if (recvSize == SOCKET_ERROR)
			throw std::runtime_error("Error while receiving response");
		else if (!recvSize)
			throw std::runtime_error("The socket has been closed");
		receivedBytes += recvSize;
	}

	BinaryRconPacket binaryResponse(binaryRconPacketHeader, responseBodyBuf);

	if (!binaryResponse.isValid())
		throw std::runtime_error("Received an invalid packet");

	TextRconPacket textResponse(binaryResponse);

	if (m_trafficLog)
		m_trafficLog->onPacketReceived(textResponse);

	return textResponse.m_data;
}
