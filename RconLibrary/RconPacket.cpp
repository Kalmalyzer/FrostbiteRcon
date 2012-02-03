
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "RconPacket.h"

#include <cstring>

#include <Windows.h>

Words createWords(const char* word0)
{
	const char* words[1] = { word0 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1)
{
	const char* words[2] = { word0, word1 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2)
{
	const char* words[3] = { word0, word1, word2 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2, const char* word3)
{
	const char* words[4] = { word0, word1, word2, word3 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4)
{
	const char* words[5] = { word0, word1, word2, word3, word4 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5)
{
	const char* words[6] = { word0, word1, word2, word3, word4, word5 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6)
{
	const char* words[7] = { word0, word1, word2, word3, word4, word5, word6 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6, const char* word7)
{
	const char* words[8] = { word0, word1, word2, word3, word4, word5, word6, word7 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6, const char* word7, const char* word8)
{
	const char* words[9] = { word0, word1, word2, word3, word4, word5, word6, word7, word8 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6, const char* word7, const char* word8, const char* word9)
{
	const char* words[10] = { word0, word1, word2, word3, word4, word5, word6, word7, word8, word9 };
	return createWords(sizeof words / sizeof words[0], words);
}

Words createWords(unsigned int numWords, const char** words)
{
	Words outWords;
	for (unsigned int word = 0; word < numWords; ++word)
		outWords.push_back(words[word]);

	return outWords;
}

std::string toString(const Words& words)
{
	std::string result;
	for (Words::const_iterator word = words.begin(), end = words.end(); word != end; ++word)
	{
		if (word != words.begin())
			result += " ";

		bool needsQuotes = (word->find(' ') != std::string::npos);

		if (needsQuotes)
			result += "\"";

		result += *word;

		if (needsQuotes)
			result += "\"";
	}

	return result;

}

std::string toString(const TextRconPacket& packet)
{
	return packet.toString();
}

std::string toString(const BinaryRconPacket& packet)
{
	return TextRconPacket(packet).toString();
}


TextRconPacket::TextRconPacket(const BinaryRconPacket& binaryRconPacket)
{
	if (!binaryRconPacket.isValid())
		throw std::runtime_error("Invalid binary packet");

	unsigned int offset = 0;
	uint32_t sequence = binaryRconPacket.readU32(offset);
	offset += 4;

	m_sequence = sequence & BinaryRconPacket::SequenceMask;
	m_originatedOnClient = ((sequence & BinaryRconPacket::OriginatedOnClientFlag) ? true : false);
	m_isResponse = ((sequence & BinaryRconPacket::IsResponseFlag) ? true : false);

	uint32_t packetSize = binaryRconPacket.readU32(offset);
	offset += 4;

	uint32_t numWords = binaryRconPacket.readU32(offset);
	offset += 4;

	for (unsigned int  word = 0; word < numWords; ++word)
	{
		uint32_t wordSize = binaryRconPacket.readU32(offset);
		offset += 4;
		m_data.push_back(binaryRconPacket.getConstCharPtr(offset));
		offset += wordSize + 1;
	}
}

TextRconPacket::TextRconPacket(bool originatedOnClient, bool isResponse, uint32_t sequence, const Words& words)
	: m_originatedOnClient(originatedOnClient)
	, m_isResponse(isResponse)
	, m_sequence(sequence)
	, m_data(words)
{
}

bool TextRconPacket::isValid() const
{
	if (m_sequence & ~BinaryRconPacket::SequenceMask)
		return false;

	unsigned int  size = 8;
	for (unsigned int  currentWord = 0; currentWord < m_data.size(); ++currentWord)
	{
		size += m_data[currentWord].size() + 4 + 1;

		if (size > BinaryRconPacket::MaxPacketSize)
			return false;
	}

	return true;
}

std::string TextRconPacket::toString() const
{
	char result[BinaryRconPacket::MaxPacketSize + 64];
	sprintf(result, "IsResponse: %s OriginatedOnClient: %s Sequence: %d %s",
		m_isResponse ? "true" : "false",
		m_originatedOnClient ? "true" : "false",
		m_sequence,
		::toString(m_data).c_str());

	return result;
}

BinaryRconPacketHeader::BinaryRconPacketHeader(const uint8_t buf[BinaryRconPacketHeader::Size])
{
	memcpy(m_data, buf, sizeof m_data);
}

bool BinaryRconPacketHeader::isValid() const
{
	uint32_t packetSize = getPacketSize();
	return (packetSize >= Size 
		&& packetSize <= BinaryRconPacket::MaxPacketSize);
}

uint32_t BinaryRconPacketHeader::getPacketSize() const
{
	uint32_t packetSize = m_data[4] & 0xff
		| ((m_data[5] & 0xff) << 8)
		| ((m_data[6] & 0xff) << 16)
		| ((m_data[7] & 0xff) << 24);
	return packetSize;
}

BinaryRconPacket::BinaryRconPacket(const TextRconPacket& textRconPacket)
{
	if (!textRconPacket.isValid())
		throw std::runtime_error("Invalid text packet");

	uint32_t sequence = textRconPacket.m_sequence
		| (textRconPacket.m_originatedOnClient ? OriginatedOnClientFlag : 0)
		| (textRconPacket.m_isResponse ? IsResponseFlag : 0);

	uint32_t packetSize = 12;

	uint32_t numWords = textRconPacket.m_data.size();

	for (unsigned int  word = 0; word < numWords; ++word)
		packetSize += textRconPacket.m_data[word].size() + 4 + 1;

	appendU32(sequence);
	appendU32(packetSize);
	appendU32(numWords);

	for (unsigned int  word = 0; word < numWords; ++word)
		appendWord(textRconPacket.m_data[word]);
}

BinaryRconPacket::BinaryRconPacket(const BinaryRconPacketHeader& binaryRconPacketHeader, const uint8_t* binaryRconPacketBody)
{
	for (unsigned int headerSize = 0; headerSize < BinaryRconPacketHeader::Size; ++headerSize)
		appendU8(binaryRconPacketHeader.m_data[headerSize]);

	unsigned int packetSize = binaryRconPacketHeader.getPacketSize();

	for (unsigned int offset = 0; offset < packetSize - BinaryRconPacketHeader::Size; ++offset)
		appendU8(binaryRconPacketBody[offset]);
}

bool BinaryRconPacket::isValid() const
{
	if (m_data.size() < BinaryRconPacketHeader::Size
		|| m_data.size() > MaxPacketSize)
		return false;

	unsigned int  offset = 4;
	unsigned int  packetSize = readU32(offset);
	offset += 4;
	unsigned int  numWords = readU32(offset);
	offset += 4;

	if (packetSize != m_data.size())
		return false;

	for (unsigned int  currentWord = 0; currentWord < numWords; ++currentWord)
	{
		unsigned int  wordLength = readU32(offset);
		offset += 4;

		if (offset >= packetSize)
			return false;

		if (wordLength > packetSize)
			return false;

		while (wordLength--)
		{
			if (readU8(offset) == 0)
				return false;
			offset++;
		}

		if (offset >= packetSize)
			return false;

		if (readU8(offset) != 0)
			return false;
		offset++;
	}

	if (offset != packetSize)
		return false;

	return true;
}

void BinaryRconPacket::getBuffer(const uint8_t*& data, unsigned int & length)
{
	data = m_data.data();
	length = m_data.size();
}

uint32_t BinaryRconPacket::readU32(unsigned int  offset) const
{
	uint32_t value = (m_data[offset])
					| (m_data[offset + 1] << 8)
					| (m_data[offset + 2] << 16)
					| (m_data[offset + 3] << 24);
	return value;
}

uint8_t BinaryRconPacket::readU8(unsigned int offset) const
{
	return m_data[offset];
}

const char* BinaryRconPacket::getConstCharPtr(unsigned int offset) const
{
	return reinterpret_cast<const char*>(&m_data[offset]);
}

void BinaryRconPacket::appendU32(uint32_t value)
{
	m_data.push_back(value & 0xff);
	m_data.push_back((value >> 8) & 0xff);
	m_data.push_back((value >> 16) & 0xff);
	m_data.push_back((value >> 24) & 0xff);
}

void BinaryRconPacket::appendU8(uint8_t value)
{
	m_data.push_back(value);
}

void BinaryRconPacket::appendWord(const std::string& word)
{
	uint32_t wordLength = word.size();
	appendU32(wordLength);
	for (unsigned int  offset = 0; offset < wordLength; ++offset)
		m_data.push_back(word[offset]);

	m_data.push_back(0);
}

