#pragma once
#include <string>

//нумерованый список
enum TokenType {
    TT_UNKNOWN,     //неизвестная лексема
    TT_KEYWORD,     //ключевое слово (PROGRAM, INTEGER, REAL, END, CALL)
    TT_IDENTIFIER,  //идентификатор (Id)
    TT_INTEGER,     //целое число (Const)
    TT_REAL,        //вещественное число (Const)
    TT_ASSIGN,      //оператор присваивания "="
    TT_PLUS,        //"+"
    TT_MINUS,       //"-"
    TT_COMMA,       //","
    TT_LPAREN,      //"("
    TT_RPAREN,      //")"
    TT_ERROR        //ошибка в лексеме
};

class Token {
private:
    std::string lexeme;
    TokenType type;
    int line;

public:
    Token() : lexeme(""), type(TT_UNKNOWN), line(1) {}

    Token(const std::string& lex, TokenType t, int ln = 1)
        : lexeme(lex), type(t), line(ln) {
    }

    //Геттеры
    std::string getLexeme() const { return lexeme; }
    std::string getKey() const { return lexeme; }
    TokenType getType() const { return type; }
    int getLine() const { return line; }

    //Сеттеры
    void setLexeme(const std::string& lex) { lexeme = lex; }
    void setType(TokenType t) { type = t; }
    void setLine(int ln) { line = ln; }

    //Преобразование типа в строку для вывода
    std::string typeToString() const {
        switch (type) {
        case TT_KEYWORD: return "KEYWORD";
        case TT_IDENTIFIER: return "IDENTIFIER";
        case TT_INTEGER: return "INTEGER";
        case TT_REAL: return "REAL";
        case TT_ASSIGN: return "ASSIGN";
        case TT_PLUS: return "PLUS";
        case TT_MINUS: return "MINUS";
        case TT_COMMA: return "COMMA";
        case TT_LPAREN: return "LPAREN";
        case TT_RPAREN: return "RPAREN";
        case TT_ERROR: return "ERROR";
        default: return "UNKNOWN";
        }
    }
};