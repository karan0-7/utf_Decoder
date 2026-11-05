#include <iostream>
#include "./decoder/decoder.h"
#include <vector>

int main()
{

    std::vector<int> characters;
    decode("input.txt", characters);
    for (int character : characters)
    {
        std::cout << "Decimal value: " << character << '\n';
    }
    return 0;
}
