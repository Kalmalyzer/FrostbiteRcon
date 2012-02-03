
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "AsynchronousServerConnectionBase.h"
#include "ServerConnectionTrafficBase.h"

#include <WinSock2.h>



AsynchronousServerConnectionBase::AsynchronousServerConnectionBase(const char* host, unsigned int port, ServerConnectionTrafficBase* trafficLog)
	: m_trafficLog(trafficLog)
	, m_receivedPacketBytes(0)
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

	if (m_trafficLog)
		m_trafficLog->onHostLookup(host);

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

	if (m_trafficLog)
		m_trafficLog->onConnecting(host, port);

	if (connect(m_socket, reinterpret_cast<const sockaddr*>(&sin), sizeof sin) != 0)
	{
		WSACleanup();
		throw std::runtime_error("Error while connecting to remote host");
	}

	if (m_trafficLog)
		m_trafficLog->onConnected();

	unsigned long nonBlockingEnabled = 1;
	error = ioctlsocket(m_socket, FIONBIO, &nonBlockingEnabled);
	if (error != 0)
	{
		closesocket(m_socket);
		WSACleanup();
		throw std::runtime_error("Unable to set socket to nonblocking mode");
	}
}

AsynchronousServerConnectionBase::~AsynchronousServerConnectionBase()
{
	if (m_socket)
	{
		closesocket(m_socket);

		if (m_trafficLog)
			m_trafficLog->onDisconnected();
	}

	WSACleanup();
}

void AsynchronousServerConnectionBase::update()
{
	if (!m_socket)
		return;

	int recvSize = recv(m_socket, reinterpret_cast<char*>(&m_receiveBuffer[m_receivedPacketBytes]), BinaryRconPacket::MaxPacketSize - m_receivedPacketBytes, 0);
	if (recvSize == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
			throw std::runtime_error("Error while reading from socket");
	}
	else if (recvSize == 0)
	{
		closesocket(m_socket);
		if (m_trafficLog)
			m_trafficLog->onDisconnected();
	}
	else
		m_receivedPacketBytes += recvSize;

	while (true)
	{
		if (m_receivedPacketBytes >= BinaryRconPacketHeader::Size)
		{
			BinaryRconPacketHeader binaryRconPacketHeader(m_receiveBuffer);
			if (!binaryRconPacketHeader.isValid())
				throw std::runtime_error("Received an invalid packet");

			uint32_t binaryRconPacketSize = binaryRconPacketHeader.getPacketSize();
			uint32_t binaryRconBodySize = binaryRconPacketSize - BinaryRconPacketHeader::Size;

			if (m_receivedPacketBytes >= binaryRconPacketSize)
			{
				BinaryRconPacket binaryRconPacket(binaryRconPacketHeader, &m_receiveBuffer[BinaryRconPacketHeader::Size]);

				if (!binaryRconPacket.isValid())
					throw std::runtime_error("Received an invalid packet");

				TextRconPacket textRconPacket(binaryRconPacket);

				if ((textRconPacket.m_isResponse && !textRconPacket.m_originatedOnClient)
				    || (!textRconPacket.m_isResponse && textRconPacket.m_originatedOnClient))
					throw std::runtime_error("Received an invalid packet");

				if (m_trafficLog)
					m_trafficLog->onPacketReceived(textRconPacket);

				if (textRconPacket.m_isResponse)
					onServerResponse(textRconPacket.m_sequence, textRconPacket.m_data);
				else
					onServerRequest(textRconPacket.m_sequence, textRconPacket.m_data);

				m_receivedPacketBytes -= binaryRconPacketSize;
				memmove(m_receiveBuffer, m_receiveBuffer + binaryRconPacketSize, m_receivedPacketBytes);
			}
			else
				break;
		}
		else
			break;
	}

}

void AsynchronousServerConnectionBase::sendRequest(uint32_t sequence, Words words)
{
	TextRconPacket textRequest(true, false, sequence, words);

	BinaryRconPacket binaryRequest(textRequest);

	const uint8_t* requestData;
	unsigned int requestLength;
	binaryRequest.getBuffer(requestData, requestLength);

	int bytesSent = send(m_socket, reinterpret_cast<const char*>(requestData), requestLength, 0);

	if (bytesSent != requestLength)
		throw std::runtime_error("Error when sending request");

	if (m_trafficLog)
		m_trafficLog->onPacketSent(textRequest);
}

void AsynchronousServerConnectionBase::sendResponse(uint32_t sequence, Words words)
{
	TextRconPacket textResponse(true, false, sequence, words);

	BinaryRconPacket binaryResponse(textResponse);

	const uint8_t* responseData;
	unsigned int responseLength;
	binaryResponse.getBuffer(responseData, responseLength);

	int bytesSent = send(m_socket, reinterpret_cast<const char*>(responseData), responseLength, 0);

	if (bytesSent != responseLength)
		throw std::runtime_error("Error when sending response");

	if (m_trafficLog)
		m_trafficLog->onPacketSent(textResponse);
}
