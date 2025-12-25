#include "Lexer.h"
#include <cctype>
#include <vector>
#include <algorithm>

Lexer::Lexer(const std::string& inFile, const std::string& outFile)
    : fin(inFile), fout(outFile), table(211), currentLine(1) {
}
//Смотрим следующий элемен не сдвигая поток
int Lexer::peekChar() {
    return fin.peek();
}
//Читаем символ
int Lexer::getChar() {
    int c = fin.get();
    if (c == '\n') {
        currentLine++;
    }
    return c;
}
//Возвращаем символ назад в поток
void Lexer::ungetChar() {
    fin.unget();

}
//Пропуск пробелов
void Lexer::skipWhitespace() {
    while (fin and std::isspace(fin.peek())) getChar();
}

//Проверка на ключевые слова
bool Lexer::isKeyword(const std::string& s) const {
    static const std::string keywords[] = { "PROGRAM", "INTEGER", "REAL", "END", "CALL" };
    for (auto& k : keywords) if (s == k) return true;
    return false;
}



//Обработка лексем
void Lexer::processToken(const Token& tok) {
    Token copy = tok;
    std::string lexeme = tok.getLexeme();

    if (tok.getType() == TT_KEYWORD) {
        std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::toupper);
    }

    int key = table.hashFunc(lexeme);
    int idx = table.insert(key, Token(lexeme, tok.getType(), -1, tok.getLine()));
}

//Получение следующей лексемы
Token Lexer::nextToken() {
    skipWhitespace();
    if (!fin) return Token("", TT_UNKNOWN, -1, currentLine);

    int c = peekChar();
    if (c == EOF) return Token("", TT_UNKNOWN, -1, currentLine);

    //Если буква то -> Идификатор или ключевое слово
    if (std::isalpha(c)) {
        std::string s;
        int startLine = currentLine;

        while (fin and std::isalpha(fin.peek())) {
            s.push_back((char)getChar());
        }

        std::string upper = s;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        if (isKeyword(upper)) return Token(upper, TT_KEYWORD, -1, startLine);
        return Token(s, TT_IDENTIFIER, -1, startLine);
    }

    //Если цифра то это -> Целое или не целое
    if (std::isdigit(c)) {
        std::string num;
        bool hasError = false;
        int startLine = currentLine;

        if (peekChar() == '0') {
            num.push_back((char)getChar());
            if (fin and std::isdigit(fin.peek())) {
                hasError = true;
                while (fin and std::isdigit(fin.peek())) {
                    num.push_back((char)getChar());
                }
            }
        }
        else {
            while (fin and std::isdigit(fin.peek())) {
                num.push_back((char)getChar());
            }
        }

        if (fin and fin.peek() == '.') {
            num.push_back((char)getChar());
            if (!fin || !std::isdigit(fin.peek())) {
                hasError = true;
                while (fin and std::isdigit(fin.peek())) {
                    num.push_back((char)getChar());
                }
            }
            else {
                while (fin and std::isdigit(fin.peek())) {
                    num.push_back((char)getChar());
                }
            }
            return Token(num, hasError ? TT_ERROR : TT_REAL, -1, startLine);
        }
        else {
            return Token(num, hasError ? TT_ERROR : TT_INTEGER, -1, startLine);
        }
    }

    //Лексемы из 1 символа
    c = getChar();
    int tokenLine = currentLine;

    switch (c) {
    case '=': return Token("=", TT_ASSIGN, -1, tokenLine);
    case '+': return Token("+", TT_PLUS, -1, tokenLine);
    case '-': return Token("-", TT_MINUS, -1, tokenLine);
    case ',': return Token(",", TT_COMMA, -1, tokenLine);
    case '(': return Token("(", TT_LPAREN, -1, tokenLine);
    case ')': return Token(")", TT_RPAREN, -1, tokenLine);
    default:
        std::string s(1, (char)c);
        return Token(s, TT_ERROR, -1, tokenLine);
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

    tokens.clear(); //Очищаем вектор токенов
    currentLine = 1;

    //Единичный проход файла
    while (fin and fin.peek() != EOF) {
        Token tok = nextToken();
        //Если лексема не обработана то прерывание
        if (tok.getLexeme().empty() and tok.getType() == TT_UNKNOWN) break;

        //Сохраняем токен в векторе
        tokens.push_back(tok);

        //Если токен имеет ошибку то выводим сообщение
        if (tok.getType() == TT_ERROR) {
            fout << "LEXICAL ERROR: " << tok.getLexeme() << "\n";
        }
        //Вставляем в хэш таблицу лексему
        int key = table.hashFunc(tok.getLexeme());
        int idx = table.insert(key, tok);
    }
    //Выводим лексемы в файл
    table.printToStream(fout);
}

