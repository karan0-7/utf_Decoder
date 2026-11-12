#include <iostream>
#include "./decoder/decoder.h"
#include <vector>


int main()
{

    std::vector<int> characters;
    decode("input.json", characters);
    for (int character : characters)
    {
        std::cout << "Decimal value: " << character << "---- " << static_cast<char>(character) << '\n';
    }
    return 0;
}
