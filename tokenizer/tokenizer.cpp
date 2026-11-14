#include <iostream>
#include <variant>
#include "./tokenizer.h"
#include <vector>
#include <array>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <type_traits>

std::vector<Token> tokens;

const std::vector<Token> &getTokens()
{
    return tokens;
}

static const char *tokenTypeName(tokenType type)
{
    switch (type)
    {
    case tokenType::STRUCTURAL:
        return "STRUCTURAL";
    case tokenType::STRING:
        return "STRING";
    case tokenType::NUMBER:
        return "NUMBER";
    case tokenType::BOOLEAN:
        return "BOOLEAN";
    }
    return "UNKNOWN";
}

// Prints one line per token:  [index] TYPE      value
void logTokens()
{
    std::cout << "Tokens (" << tokens.size() << "):\n";

    size_t index = 0;
    for (const Token &token : tokens)
    {
        std::visit([index](const auto &t)
                   {
            std::cout << "  [" << index << "] " << std::left << std::setw(11)
                      << tokenTypeName(t.type);

            using T = std::decay_t<decltype(t)>;
            if constexpr (std::is_same_v<T, StringToken>)
                std::cout << '"' << t.value << '"';
            else if constexpr (std::is_same_v<T, BooleanToken>)
                std::cout << (t.value ? "true" : "false");
            else
                std::cout << t.value;   // char or int
            std::cout << '\n'; },
                   token);
        ++index;
    }
}

bool processStringToken(int i);
bool processStructuralToken(int i);
bool processNumberToken(int i);
bool processBooleanToken(int i);


using Processor = bool (*)(int);

std::array<Processor, 4> processors = {
    processStringToken,
    processNumberToken, // must run before structural so it can flush on ',' '}' etc.
    processStructuralToken,
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

    // end of input: flush a number that is still being collected
    processNumberToken(EOF);
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

    bool found = std::find(structuralCharacters.begin(), structuralCharacters.end(), ch) != structuralCharacters.end();
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

    if (i >= 0 && std::isdigit(i))
    {
        num += static_cast<char>(i);
        return true;
    }

    if (!num.empty())
    {
        // number ended: emit it, but do NOT consume this character,
        // so the next processor (structural, boolean, ...) still sees it
        tokens.push_back(NumberToken{tokenType::NUMBER, std::stoi(num)});
        num.clear();
    }
    return false;
}

bool processBooleanToken(int i)
{
    static std::string word = "";

    char c = static_cast<char>(i);

    if (word.empty())
    {
        // a boolean can only start with 't' or 'f'
        if (c == 't' || c == 'f')
        {
            word += c;
            return true;
        }
        return false;
    }

    word += c;
    const std::string target = (word[0] == 't') ? "true" : "false";

    if (word == target)
    {
        tokens.push_back(BooleanToken{tokenType::BOOLEAN, word == "true"});
        word.clear();
        return true;
    }

    if (target.compare(0, word.length(), word) == 0)
    {
        // still a valid prefix of "true" / "false", keep collecting
        return true;
    }

    // mismatch (e.g. "tru!" or "fax"): drop the partial word
    word.clear();
    return false;
}
