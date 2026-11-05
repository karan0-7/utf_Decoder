#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <vector>

class LeaderByte
{
private:
    enum LEADER_BYTE_TYPE
    {
        ONE_BYTE = 1,
        TWO_BYTE,
        THREE_BYTE,
        FOUR_BYTE,
        INVALID
    };

    bool isOneByteLeader(uint8_t byte)
    {
        uint8_t mask = 0x80;
        uint8_t pattern = 0x00;

        return (byte & mask) == pattern;
    }

    bool isTwoByteLeader(uint8_t byte)
    {
        uint8_t mask = 0xE0;
        uint8_t pattern = 0xC0;

        return (byte & mask) == pattern;
    }

    bool isThreeByteLeader(uint8_t byte)
    {
        uint8_t mask = 0xF0;
        uint8_t pattern = 0xE0;

        return (byte & mask) == pattern;
    }

    bool isFourByteLeader(uint8_t byte)
    {
        uint8_t mask = 0xF8;
        uint8_t pattern = 0xF0;

        return (byte & mask) == pattern;
    }

    LEADER_BYTE_TYPE getLeaderByteType(uint8_t &byte)
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
        else
        {
            return LEADER_BYTE_TYPE::INVALID;
        }
    }

    uint8_t extractOneByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x7F;
    }

    uint8_t extractTwoByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x1F;
    }

    uint8_t extractThreeByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x0F;
    }

    uint8_t extractFourByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x07;
    }

    uint8_t extractLeaderByteDataBits(uint8_t byte, LEADER_BYTE_TYPE type)
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
        case LEADER_BYTE_TYPE::INVALID:
            break;
        }
        return 0;
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
        case LEADER_BYTE_TYPE::INVALID:
            break;
        }
        return -1;
    }

public:
    struct LeaderByteInfo
    {
        bool isValid;
        int numberOfFollowers;
        uint8_t dataBits;
    };
    LeaderByteInfo processLeaderByte(uint8_t byte)
    {
        LeaderByteInfo info{false, 0, 0X00};
        LEADER_BYTE_TYPE type = getLeaderByteType(byte);
        if (type == LEADER_BYTE_TYPE::INVALID)
        {
            return info;
        }
        int numberOfFollowers = getNumberOfFollowersLeaderByte(type);
        uint8_t dataBits = extractLeaderByteDataBits(byte, type);
        info = {true, numberOfFollowers, dataBits};
        return info;
    }
};

class FollowerByte
{
private:
    bool isValidByte(uint8_t byte)
    {
        uint8_t mask = 0xC0;
        uint8_t pattern = 0x80;

        return (byte & mask) == pattern;
    }

    uint8_t getDataBits(uint8_t byte)
    {
        return byte & 0x3F;
    }

public:
    struct FollowerByteInfo
    {
        bool isValid;
        uint8_t dataBits;
    };
    FollowerByteInfo processFollowerByte(uint8_t byte)
    {
        FollowerByteInfo info{false, 0X00};
        bool isValid = isValidByte(byte);

        if (!isValid)
        {
            return info;
        }
        info.isValid = true;
        info.dataBits = getDataBits(byte);
        return info;
    }
};

enum BYTE_TYPE
{
    LEADER,
    FOLLOWER
};

BYTE_TYPE currentByteType = BYTE_TYPE::LEADER;
int totalFollowerBytes = 0;

int generateCharacter(std::vector<uint8_t> characterBytes)
{
    int sum = 0;

    for (uint8_t character : characterBytes)
    {
        sum = (sum << 6) | character;
    }
    return sum;
}

void decode(std::string fileName, std::vector<int> &characters)
{
    std::ifstream file{fileName, std::ios::binary};

    if (!file)
    {
        // throw error here
    }
    char c;
    std::string currentChar = "";
    LeaderByte leaderByte;
    FollowerByte followerByte;
    std::vector<uint8_t> characterBytes;
    while (file.get(c))
    {
        uint8_t byte = c;
        if (currentByteType == BYTE_TYPE::LEADER)
        {
            LeaderByte::LeaderByteInfo byteInfo = leaderByte.processLeaderByte(byte);

            if (!byteInfo.isValid)
            {
                characters.push_back(0xFFFD);
                continue;
            }

            std::cout << "Leader byte info -> "
                      << "isValid: " << std::boolalpha << byteInfo.isValid << std::noboolalpha
                      << ", numberOfFollowers: " << byteInfo.numberOfFollowers
                      << ", dataBits: " << static_cast<int>(byteInfo.dataBits)
                      << '\n';

            if (byteInfo.numberOfFollowers == 0)
            {
                characters.push_back(byteInfo.dataBits);
            }
            else
            {
                characterBytes.push_back(byteInfo.dataBits);
                currentByteType = BYTE_TYPE::FOLLOWER;
                totalFollowerBytes = byteInfo.numberOfFollowers;
            }
        }
        else if (currentByteType == BYTE_TYPE::FOLLOWER)
        {
            FollowerByte::FollowerByteInfo byteInfo = followerByte.processFollowerByte(byte);

            std::cout << "Follower byte info -> "
                      << "isValid: " << std::boolalpha << byteInfo.isValid << std::noboolalpha
                      << ", dataBits: " << static_cast<int>(byteInfo.dataBits)
                      << '\n';

            if (!byteInfo.isValid)
            {
                characters.push_back(0xFFFD);
                characterBytes = {};
                currentByteType = BYTE_TYPE::LEADER;
                totalFollowerBytes = 0;
                continue;
            }

            characterBytes.push_back(byteInfo.dataBits);
            totalFollowerBytes--;

            if (totalFollowerBytes == 0)
            {
                int character = generateCharacter(characterBytes);
                characters.push_back(character);
                currentByteType = BYTE_TYPE::LEADER;
                characterBytes = {};
            }
        }
    }

    if (currentByteType == BYTE_TYPE::FOLLOWER)
    {
        characters.push_back(0xFFFD); // incomplete character at end of input
        characterBytes = {};
        currentByteType = BYTE_TYPE::LEADER;
        totalFollowerBytes = 0;
    }
}