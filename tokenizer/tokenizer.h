#pragma once

#include <string>
#include <variant>
#include <vector>

enum class tokenType
{
    STRUCTURAL = 1,
    STRING,
    NUMBER,
    BOOLEAN
};

struct StructuralToken
{
    tokenType type;
    char value;
};

struct StringToken
{
    tokenType type;
    std::string value;
};

struct NumberToken
{
    tokenType type;
    int value;
};

struct BooleanToken
{
    tokenType type;
    bool value;
};

using Token = std::variant<StructuralToken, StringToken, NumberToken, BooleanToken>;

void tokenize(const std::vector<int> &characters);
const std::vector<Token> &getTokens();
void logTokens();