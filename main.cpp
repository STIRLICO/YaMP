#include "Lexer.h"
#include "Synt.h"
#include <iostream>
#include <fstream>
#include <vector>

int main() {
    std::string inFile = "input.txt";
    std::string lexerOutFile = "output.txt";
    std::string parserOutFile = "output2.txt";

    //Лексический анализ
    Lexer lexer(inFile, lexerOutFile);
    lexer.run();

    //Получаем токены из лексического анализатора в порядке появления
    std::vector<Token> tokens = lexer.getTokens();

    //Отбраковываем токены ошибки
    std::vector<Token> validTokens;
    for (const auto& token : tokens) {
        if (token.getType() != TT_ERROR and token.getType() != TT_UNKNOWN) {
            validTokens.push_back(token);
        }
    }

    //Синтаксический анализ
    std::ofstream parserOutput(parserOutFile);
    Synt parser(validTokens, parserOutput);
    parser.synt();

    std::cout << "Lexical output written to " << lexerOutFile << "\n";
    std::cout << "Synt output written to " << parserOutFile << "\n";

}