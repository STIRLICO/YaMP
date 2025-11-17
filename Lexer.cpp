#include "Lexer.h"
#include <cctype>
#include <vector>
#include <algorithm>

Lexer::Lexer(const std::string& inFile, const std::string& outFile)
    : fin(inFile), fout(outFile), table(211) {
}

//Смотрим следующий элемен не сдвигая поток
int Lexer::peekChar() {
    return fin.peek();
}
//Читаем символ
int Lexer::getChar() {
    return fin.get();
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

    //Переводим ключевые слова в вехний регистр
    if (tok.getType() == TT_KEYWORD) {
        std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::toupper);
    }

    //Вычилсяем хэш функцию
    int key = table.hashFunc(lexeme);
    int idx = table.insert(key, Token(lexeme, tok.getType()));

}

//Получение следующей лексемы
Token Lexer::nextToken() {
    skipWhitespace();
    if (!fin) return Token("", TT_UNKNOWN);//Файл кончился

    int c = peekChar();
    if (c == EOF) return Token("", TT_UNKNOWN);//Мы в последнем символе

    // Если буква то -> Идификатор или ключевое слово
    if (std::isalpha(c)) {
        std::string s;
        //Считываем пока файл не кончился и следующий сивол - буква
        while (fin and std::isalpha(fin.peek())) {
            s.push_back((char)getChar());
        }

        std::string upper = s;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        //Если верхний регистр совпадает с ключевым словом то это оно и есть
        if (isKeyword(upper)) return Token(upper, TT_KEYWORD);
        return Token(s, TT_IDENTIFIER); //Иначе идификатор
    }

    //Если цифра то это -> Целое или не целое
    if (std::isdigit(c)) {
        std::string num;
        bool hasError = false;

        //Если ноль в начале то собираем число полностью и помечаем как ошибку
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
            //Собираем число
            while (fin and std::isdigit(fin.peek())) {
                num.push_back((char)getChar());
            }
        }

        if (fin and fin.peek() == '.') {
            num.push_back((char)getChar()); //Содержит ноль и дальше нет цифры то ошибка
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
            //Если число с ошибкой то ставим тип ошибки
            return Token(num, hasError ? TT_ERROR : TT_REAL);
        }
        else {
            return Token(num, hasError ? TT_ERROR : TT_INTEGER);
        }
    }

    //Лексемы из 1 символа
    c = getChar();
    switch (c) {
    case '=': return Token("=", TT_ASSIGN);
    case '+': return Token("+", TT_PLUS);
    case '-': return Token("-", TT_MINUS);
    case ',': return Token(",", TT_COMMA);
    case '(': return Token("(", TT_LPAREN);
    case ')': return Token(")", TT_RPAREN);
    default:
        std::string s(1, (char)c);
        return Token(s, TT_ERROR);
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

    //Единичный проход файла
    while (fin and fin.peek() != EOF) {
        Token tok = nextToken();
        //Если лексема не обработана то прерыване
        if (tok.getLexeme().empty() and tok.getType() == TT_UNKNOWN) break;

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

