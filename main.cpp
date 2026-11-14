#include "./decoder/decoder.h"
#include "./tokenizer/tokenizer.h"
#include <vector>

int main()
{
    std::vector<int> characters;
    decode("input.json", characters);

    tokenize(characters);
    logTokens();
    return 0;
}
