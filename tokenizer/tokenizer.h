#include <string>

enum tokenType
{
    STRUCTURAL = 1,
    STRING,
    NUMBER,
    BOOLEAN
};

struct StructuralToken
{
    char value;
};

struct StringToken
{
    std::string value;
};

struct NumberToken
{
    int value;
};

struct BooleanToken
{
    bool value;
};