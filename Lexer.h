#pragma once
#include "Token.h"
#include "HashTable.h"
#include <fstream>
#include <string>
#include <vector>

class Lexer {
public:
	Lexer(const std::string& inFile, const std::string& outFile);
	void run(); //Выполняем 1 проход
	std::vector<Token> getTokens() const { return tokens; }

private:
	std::ifstream fin;
	std::ofstream fout;
	HashTable<Token> table;
	std::vector<Token> tokens; //Вектор для хранения токенов в порядке появления
	int currentLine;

	int hashFunc(const std::string& s) const;
	void processToken(const Token& tok);

	//Чтение по символам
	int peekChar();
	int getChar();
	void ungetChar();

	//Доп функции для ДКА
	void skipWhitespace();
	Token nextToken();
	bool isKeyword(const std::string& s) const;

	
};