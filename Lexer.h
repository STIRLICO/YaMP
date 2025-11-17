#pragma once
#include "Token.h"
#include "HashTable.h"
#include <fstream>
#include <string>


class Lexer {
public:
	Lexer(const std::string& inFile, const std::string& outFile);
	void run(); //Выполняем 1 проход


private:
	std::ifstream fin;
	std::ofstream fout;
	HashTable<Token> table;


	int hashFunc(const std::string& s) const; //Хэш-функция для лексемы
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