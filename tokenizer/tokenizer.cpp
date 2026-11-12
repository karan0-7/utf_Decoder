#include <iostream>
#include <variant>
#include "./tokenizer.h"
#include <vector>

std::vector<std::variant<StructuralToken, StringToken, NumberToken, BooleanToken>> tokens;

void tokenize(std::vector<int> characters)
{

    for (int i : characters)
    {
    }
}

int processStringToken(int i)
{
    static bool isStringStarted = false;
    static std::string str = "";

    if (static_cast<char>(i) == '"')
    {
        isStringStarted = true;
        return 0;
    }
    else if (isStringStarted && static_cast<char>(i) == '"')
    {
        isStringStarted = false;
        if (str.length())
        {
            StringToken stringToken{str};
            tokens.push_back(stringToken);
        }
        return 0;
    }
    else if (isStringStarted)
    {
        str += static_cast<char>(i);
    }
    return 1;
}

void processStructuralToken(int i)
{
}

void processNumberToken(int i)
{
}

void processBooleanToken(int i)
{
}