#pragma once

#include <string>

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