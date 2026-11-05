#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

enum BYTE_TYPE
{
    LEADER,
    FOLLOWER
};

enum LEADER_BYTE_TYPE
{
    ONE_BYTE,
    TWO_BYTE,
    THREE_BYTE,
    FOUR_BYTE
};

struct LeaderByteInfo
{
    bool isValid;
    int numberOfFollowers;
    int8_t dataBits;
};

BYTE_TYPE currentByteType = BYTE_TYPE::LEADER;
int totalFollowerBytes = 0;

bool isOneByteLeader(int8_t byte)
{
    int8_t mask = 0x80;
    int8_t pattern = 0x00;

    return (byte & mask) == pattern;
}

bool isTwoByteLeader(int8_t byte)
{
    int8_t mask = 0xE0;
    int8_t pattern = 0xC0;

    return (byte & mask) == pattern;
}

bool isThreeByteLeader(int8_t byte)
{
    int8_t mask = 0xF0;
    int8_t pattern = 0xE0;

    return (byte & mask) == pattern;
}

bool isFourByteLeader(int8_t byte)
{
    int8_t mask = 0xF8;
    int8_t pattern = 0xF0;

    return (byte & mask) == pattern;
}

int8_t extractOneByteLeaderDataBits(int8_t byte)
{
    return byte & 0x7F;
}

int8_t extractTwoByteLeaderDataBits(int8_t byte)
{
    return byte & 0x1F;
}

int8_t extractThreeByteLeaderDataBits(int8_t byte)
{
    return byte & 0x1F;
}

int8_t extractFourByteLeaderDataBits(int8_t byte)
{
    return byte & 0x07;
}

LEADER_BYTE_TYPE getLeaderByteType(int8_t &byte)
{
    if (isOneByteLeader(byte))
    {
        return LEADER_BYTE_TYPE::ONE_BYTE;
    }
    else if (isTwoByteLeader(byte))
    {
        return LEADER_BYTE_TYPE::TWO_BYTE;
    }
    else if (isThreeByteLeader(byte))
    {
        return LEADER_BYTE_TYPE::THREE_BYTE;
    }
    else if (isFourByteLeader(byte))
    {
        return LEADER_BYTE_TYPE::FOUR_BYTE;
    }
}

int8_t extractLeaderByteDataBits(int8_t byte, LEADER_BYTE_TYPE type)
{
    switch (type)
    {
    case LEADER_BYTE_TYPE::ONE_BYTE:
        return extractOneByteLeaderDataBits(byte);
    case LEADER_BYTE_TYPE::TWO_BYTE:
        return extractTwoByteLeaderDataBits(byte);
    case LEADER_BYTE_TYPE::THREE_BYTE:
        return extractThreeByteLeaderDataBits(byte);
    case LEADER_BYTE_TYPE::FOUR_BYTE:
        return extractFourByteLeaderDataBits(byte);
    }
}

int getNumberOfFollowersLeaderByte(LEADER_BYTE_TYPE type)
{
    switch (type)
    {
    case LEADER_BYTE_TYPE::ONE_BYTE:
        return 0;
    case LEADER_BYTE_TYPE::TWO_BYTE:
        return 1;
    case LEADER_BYTE_TYPE::THREE_BYTE:
        return 2;
    case LEADER_BYTE_TYPE::FOUR_BYTE:
        return 3;
    }
}

LeaderByteInfo processLeaderByte(int8_t byte)
{
    LeaderByteInfo info{false, 0, 0X00};
    LEADER_BYTE_TYPE type = getLeaderByteType(byte);
    if (!type)
    {
        return info;
    }
    int numberOfFollowers = getNumberOfFollowersLeaderByte(type);
    int8_t dataBits = extractLeaderByteDataBits(byte, type);
    info = {true, numberOfFollowers, dataBits};
    return info;
}

int main()
{
    std::ifstream file{"hello.json", std::ios::binary};

    if (!file)
    {
        // throw error here
    }
    char c;
    std::string currentChar = "";
    while (file.get(c))
    {
        int8_t byte = c;
        std::cout << static_cast<int>(c) << '\n';
    }
}
/*
extractDataBits -> if valid utf8 byte the returned the valid data bits from that byte
if currentbytetype is leader then we need -> numberoffollowers,mydatabits
*/