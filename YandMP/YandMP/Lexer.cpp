#include "Lexer.h"
#include <cctype>
#include <vector>
#include <algorithm>

Lexer::Lexer(const std::string& inFile, const std::string& outFile)
    : fin(inFile), fout(outFile), table(211), currentLine(1) {
}

int Lexer::peekChar() {
    return fin.peek();
}

int Lexer::getChar() {
    int c = fin.get();
    if (c == '\n') {
        currentLine++;
    }
    return c;
}

void Lexer::ungetChar() {
    fin.unget();
}

void Lexer::skipWhitespace() {
    while (fin && std::isspace(fin.peek())) getChar();
}

bool Lexer::isKeyword(const std::string& s) const {
    static const std::string keywords[] = { "PROGRAM", "INTEGER", "REAL", "END", "CALL" };
    for (auto& k : keywords) if (s == k) return true;
    return false;
}

Token Lexer::nextToken() {
    skipWhitespace();
    if (!fin) return Token("", TT_UNKNOWN, currentLine);

    int c = peekChar();
    if (c == EOF) return Token("", TT_UNKNOWN, currentLine);

    if (std::isalpha(c)) {
        std::string s;
        int startLine = currentLine;

        while (fin && std::isalpha(fin.peek())) {
            s.push_back((char)getChar());
        }

        std::string upper = s;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        if (isKeyword(upper)) return Token(upper, TT_KEYWORD, startLine);
        return Token(s, TT_IDENTIFIER, startLine);
    }

    if (std::isdigit(c)) {
        std::string num;
        bool hasError = false;
        int startLine = currentLine;

        if (peekChar() == '0') {
            num.push_back((char)getChar());
            if (fin && std::isdigit(fin.peek())) {
                hasError = true;
                while (fin && std::isdigit(fin.peek())) {
                    num.push_back((char)getChar());
                }
            }
        }
        else {
            while (fin && std::isdigit(fin.peek())) {
                num.push_back((char)getChar());
            }
        }

        if (fin && fin.peek() == '.') {
            num.push_back((char)getChar());
            if (!fin || !std::isdigit(fin.peek())) {
                hasError = true;
                while (fin && std::isdigit(fin.peek())) {
                    num.push_back((char)getChar());
                }
            }
            else {
                while (fin && std::isdigit(fin.peek())) {
                    num.push_back((char)getChar());
                }
            }
            return Token(num, hasError ? TT_ERROR : TT_REAL, startLine);
        }
        else {
            return Token(num, hasError ? TT_ERROR : TT_INTEGER, startLine);
        }
    }

    c = getChar();
    int tokenLine = currentLine;

    switch (c) {
    case '=': return Token("=", TT_ASSIGN, tokenLine);
    case '+': return Token("+", TT_PLUS, tokenLine);
    case '-': return Token("-", TT_MINUS, tokenLine);
    case ',': return Token(",", TT_COMMA, tokenLine);
    case '(': return Token("(", TT_LPAREN, tokenLine);
    case ')': return Token(")", TT_RPAREN, tokenLine);
    default:
        std::string s(1, (char)c);
        return Token(s, TT_ERROR, tokenLine);
    }
}

void Lexer::run() {
    if (!fin.is_open()) {
        std::cerr << "Cannot open input file\n";
        return;
    }
    if (!fout.is_open()) {
        std::cerr << "Cannot open output file\n";
        return;
    }

    tokens.clear();
    currentLine = 1;

    while (fin && fin.peek() != EOF) {
        Token tok = nextToken();
        if (tok.getLexeme().empty() && tok.getType() == TT_UNKNOWN) break;

        tokens.push_back(tok);

        if (tok.getType() == TT_ERROR) {
            fout << "LEXICAL ERROR: " << tok.getLexeme() << "\n";
        }
        int idx = table.insert(tok.getLexeme(), tok);
    }
    table.printToStream(fout);
}