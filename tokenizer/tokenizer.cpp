#include <iostream>
#include <variant>
#include "./tokenizer.h"
#include <vector>
#include <array>
#include <algorithm>
#include <cctype>

std::vector<std::variant<StructuralToken, StringToken, NumberToken, BooleanToken>> tokens;

bool processStringToken(int i);
bool processStructuralToken(int i);
bool processNumberToken(int i);
bool processBooleanToken(int i);


using Processor = bool (*)(int);

std::array<Processor, 4> processors = {
    processStringToken,
    processStructuralToken,
    processNumberToken,
    processBooleanToken};

void tokenize(const std::vector<int> &characters)
{
    for (int i : characters)
    {
        for (Processor processor : processors)
        {
            if (processor(i))
            {
                break;
            }
        }
    }
}

bool processStringToken(int i)
{
    static bool isStringStarted = false;
    static std::string str = "";

    char c = static_cast<char>(i);

    if (isStringStarted && c == '"')
    {
        isStringStarted = false;
        tokens.push_back(StringToken{tokenType::STRING, str});
        str.clear();
        return true;
    }
    else if (c == '"')
    {
        isStringStarted = true;
        return true;
    }
    else if (isStringStarted)
    {
        str += c;
        return true;
    }
    return false;
}

bool processStructuralToken(int i)
{
    std::array<char, 6> structuralCharacters = {'{', '}', '[', ']', ',', ':'};
    char ch = static_cast<char>(i);

    bool found = std::find(structuralCharacters.begin(), structuralCharacters.end(), ch);
    if (found)
    {
        StructuralToken st{tokenType::STRUCTURAL, ch};
        tokens.push_back(st);
        return true;
    }

    return false;
}

bool processNumberToken(int i)
{
    static std::string num = "";

    if (std::isdigit(i))
    {
        num += i;
        return true;
    }
    else
    {
        if (num.length())
        {
            NumberToken nt{tokenType::NUMBER, std::stoi(num)};
            tokens.push_back(nt);
            return true;
        }
    }
    return false;
}

bool processBooleanToken(int i)
{
    return false;
}
