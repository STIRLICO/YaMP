#pragma once
#include "Token.h"
#include "HashTable.h"
#include <fstream>
#include <string>
#include <vector>

class Lexer {
public:
    Lexer(const std::string& inFile, const std::string& outFile);
    void run();
    std::vector<Token> getTokens() const { return tokens; }

private:
    std::ifstream fin;
    std::ofstream fout;
    HashTable<Token> table;
    std::vector<Token> tokens;
    int currentLine;

    int peekChar();
    int getChar();
    void ungetChar();
    void skipWhitespace();
    Token nextToken();
    bool isKeyword(const std::string& s) const;
};