
#ifndef RconPacket_h
#define RconPacket_h

#include <vector>
#include <string>
#include <cstdint>

typedef std::string Word;
typedef std::vector<std::string> Words;

Words createWords(const char* word0);
Words createWords(const char* word0, const char* word1);
Words createWords(const char* word0, const char* word1, const char* word2);
Words createWords(const char* word0, const char* word1, const char* word2, const char* word3);
Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4);
Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5);
Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6);
Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6, const char* word7);
Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6, const char* word7, const char* word8);
Words createWords(const char* word0, const char* word1, const char* word2, const char* word3, const char* word4, const char* word5, const char* word6, const char* word7, const char* word8, const char* word9);
Words createWords(unsigned int numWords, const char** words);

class TextRconPacket;
class BinaryRconPacket;

std::string toString(const Words& words);
std::string toString(const TextRconPacket& packet);
std::string toString(const BinaryRconPacket& packet);

class TextRconPacket
{
public:
	explicit TextRconPacket(const BinaryRconPacket& binaryRconPacket);
	explicit TextRconPacket(bool originatedOnClient, bool isResponse, uint32_t sequence, const Words& words);

	bool isValid() const;

	std::string toString() const;

	bool m_originatedOnClient;
	bool m_isResponse;
	uint32_t m_sequence;

	Words m_data;
};

class BinaryRconPacketHeader
{
public:
	enum { Size = 12 };

	explicit BinaryRconPacketHeader(const uint8_t buf[Size]);

	bool isValid() const;
	uint32_t getPacketSize() const;

private:
	uint8_t m_data[12];

	friend class BinaryRconPacket;
};

class BinaryRconPacket
{
public:
	explicit BinaryRconPacket(const TextRconPacket& textRconPacket);
	explicit BinaryRconPacket(const BinaryRconPacketHeader& binaryRconPacketHeader, const uint8_t* binaryRconPacketBody);

	enum { MaxPacketSize = 16384 };
	enum { SequenceMask = 0x3fffffff };
	enum { OriginatedOnClientFlag = 0x80000000U };
	enum { IsResponseFlag = 0x40000000 };

	bool isValid() const;

	void getBuffer(const uint8_t*& data, unsigned int& length);

private:
	uint32_t readU32(unsigned int offset) const;
	uint8_t readU8(unsigned int offset) const;
	const char* getConstCharPtr(unsigned int offset) const;
	void appendU8(uint8_t value);
	void appendU32(uint32_t value);
	void appendWord(const std::string& word);

	std::vector<uint8_t> m_data;

	friend class TextRconPacket;
};

#endif
