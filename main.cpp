#include "Lexer.h"
#include <iostream>

int main() {
    std::string inFile = "input.txt";
    std::string outFile = "output.txt";

    Lexer lexer(inFile, outFile);
    lexer.run();

    std::cout << "Output written";
    return 0;
}


