#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace
{

class LeaderByte
{
public:
    enum LEADER_BYTE_TYPE
    {
        ONE_BYTE = 1,
        TWO_BYTE,
        THREE_BYTE,
        FOUR_BYTE,
        INVALID
    };
    struct LeaderByteInfo
    {
        bool isValid;
        int numberOfFollowers;
        uint8_t rawByte;
        LEADER_BYTE_TYPE type;
    };
    LeaderByteInfo processLeaderByte(uint8_t byte)
    {
        LeaderByteInfo info{false, 0, 0X00, LEADER_BYTE_TYPE::INVALID};
        LEADER_BYTE_TYPE type = getLeaderByteType(byte);
        if (type == LEADER_BYTE_TYPE::INVALID)
        {
            return info;
        }
        int numberOfFollowers = getNumberOfFollowersLeaderByte(type);
        info = {true, numberOfFollowers, byte, type};
        return info;
    }

    static uint8_t extractLeaderByteDataBits(uint8_t byte, LEADER_BYTE_TYPE type)
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

private:
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

    LEADER_BYTE_TYPE getLeaderByteType(uint8_t byte)
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

    static uint8_t extractOneByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x7F;
    }

    static uint8_t extractTwoByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x1F;
    }

    static uint8_t extractThreeByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x0F;
    }

    static uint8_t extractFourByteLeaderDataBits(uint8_t byte)
    {
        return byte & 0x07;
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

public:
    static uint8_t getDataBits(uint8_t byte)
    {
        return byte & 0x3F;
    }
    struct FollowerByteInfo
    {
        bool isValid;
        uint8_t rawByte;
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
        info.rawByte = byte;
        return info;
    }
};

struct Character
{
    LeaderByte::LeaderByteInfo leader{};
    std::vector<FollowerByte::FollowerByteInfo> followers{};
};

enum BYTE_TYPE
{
    LEADER,
    FOLLOWER
};

bool isLeaderFollowersInRange(const Character &character)
{
    if (character.leader.type == LeaderByte::LEADER_BYTE_TYPE::TWO_BYTE)
    {
        return character.leader.rawByte >= 0xC2;
    }
    else if (character.leader.type == LeaderByte::LEADER_BYTE_TYPE::THREE_BYTE)
    {
        if (character.followers.size() < 2)
        {
            return false;
        }

        uint8_t leader = character.leader.rawByte;
        uint8_t second = character.followers[0].rawByte;

        uint8_t secondMin = 0x80;
        uint8_t secondMax = 0xBF;

        if (leader == 0xE0)
        {
            secondMin = 0xA0; // blocks overlong encodings
        }
        else if (leader == 0xED)
        {
            secondMax = 0x9F; // blocks surrogates
        }

        return second >= secondMin && second <= secondMax;
    }
    else if (character.leader.type == LeaderByte::LEADER_BYTE_TYPE::FOUR_BYTE)
    {
        if (character.followers.size() < 3)
        {
            return false;
        }

        uint8_t leader = character.leader.rawByte;
        uint8_t second = character.followers[0].rawByte;

        if (leader > 0xF4)
        {
            return false; // F5-F7 would encode above U+10FFFF
        }

        uint8_t secondMin = 0x80;
        uint8_t secondMax = 0xBF;

        if (leader == 0xF0)
        {
            secondMin = 0x90; // blocks overlong encodings
        }
        else if (leader == 0xF4)
        {
            secondMax = 0x8F; // blocks values above U+10FFFF
        }

        return second >= secondMin && second <= secondMax;
    }
    else
    {
        return false;
    }
}

int generateCharacter(const Character &character)
{
    int sum = LeaderByte::extractLeaderByteDataBits(character.leader.rawByte, character.leader.type);

    for (const FollowerByte::FollowerByteInfo &follower : character.followers)
    {
        sum = (sum << 6) | FollowerByte::getDataBits(follower.rawByte);
    }
    return sum;
}

} // namespace

void decode(std::string fileName, std::vector<int> &characters)
{
    std::ifstream file{fileName, std::ios::binary};

    if (!file)
    {
        return;
    }

    BYTE_TYPE currentExpectedByteType = BYTE_TYPE::LEADER;
    Character currentChar;
    LeaderByte leaderByte;
    FollowerByte followerByte;
    char c;
    while (file.get(c))
    {
        uint8_t byte = c;
        if (currentExpectedByteType == BYTE_TYPE::LEADER)
        {
            LeaderByte::LeaderByteInfo byteInfo = leaderByte.processLeaderByte(byte);

            if (!byteInfo.isValid)
            {
                continue;
            }

            if (byteInfo.numberOfFollowers == 0)
            {
                characters.push_back(generateCharacter(Character{byteInfo, {}}));
            }
            else
            {
                currentChar.leader = byteInfo;
                currentExpectedByteType = BYTE_TYPE::FOLLOWER;
            }
        }
        else
        {
            FollowerByte::FollowerByteInfo byteInfo = followerByte.processFollowerByte(byte);

            if (!byteInfo.isValid)
            {
                currentExpectedByteType = BYTE_TYPE::LEADER;
                currentChar = {};
                continue;
            }

            currentChar.followers.push_back(byteInfo);

            if (static_cast<int>(currentChar.followers.size()) == currentChar.leader.numberOfFollowers)
            {
                if (isLeaderFollowersInRange(currentChar))
                {
                    characters.push_back(generateCharacter(currentChar));
                }
                currentExpectedByteType = BYTE_TYPE::LEADER;
                currentChar = {};
            }
        }
    }
}
