#include "Synt.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Synt::Synt(const std::vector<Token>& tokenList, std::ofstream& output)
    : tokens(tokenList), currentTokenIndex(0), astOutput(output),
    indentLevel(0), lineNumber(0), inDescriptionsSection(true), root(nullptr) {
}

void Synt::printNumberedLine(const std::string& line) {
    astOutput << line << "\n";
}

void Synt::printNode(const std::string& nodeName) {
    std::string indentedStr;
    for (int i = 0; i < indentLevel; i++) indentedStr += "  ";
    indentedStr += nodeName;
    astOutput << indentedStr << "\n";
}

void Synt::printLeaf(const std::string& value) {
    std::string indentedStr;
    for (int i = 0; i < indentLevel; i++) indentedStr += "  ";
    indentedStr += "'" + value + "'";
    astOutput << std::setw(3) << std::right << " " << indentedStr << "\n";
    //lineNumber++;
}

//Отступ
void Synt::increaseIndent() {
    indentLevel++;
}

void Synt::decreaseIndent() {
    if (indentLevel > 0) indentLevel--;
}

//Получение данного токена
Token Synt::currentToken() const {
    if (currentTokenIndex < tokens.size()) {
        return tokens[currentTokenIndex];
    }
    return Token("", TT_UNKNOWN);
}

//Переход к следующему токену
void Synt::nextToken() {
    if (currentTokenIndex < tokens.size()) {
        currentTokenIndex++;
    }
}

//Сравнение типа токена с ожидаемым
bool Synt::match(TokenType expected) {
    if (currentTokenIndex >= tokens.size()) return false;
    if (currentToken().getType() == expected) {
        nextToken();
        return true;
    }
    return false;
}

//Сравнение ключевых слов
bool Synt::matchKeyword(const std::string& keyword) {
    if (currentTokenIndex >= tokens.size()) return false;
    if (currentToken().getType() == TT_KEYWORD && currentToken().getLexeme() == keyword) {
        nextToken();
        return true;
    }
    return false;
}

//Сообщение об ошибке
void Synt::error(const std::string& message) {
    std::stringstream ss;

    if (currentTokenIndex < tokens.size()) {
        Token token = currentToken();
        ss << "SYNTAX ERROR at line " << token.getLine()
            << " (token: '" << token.getLexeme() << "'): " << message;
    }
    else {
        //Если токены закончились
        if (!tokens.empty()) {
            Token lastToken = tokens.back();
            ss << "SYNTAX ERROR at line " << lastToken.getLine()
                << ": " << message << " (unexpected end of file)";
        }
        else {
            ss << "SYNTAX ERROR: " << message << " (file is empty)";
        }
    }

    errors.push_back(ss.str());
    syncAfterError();
}

//Восстановление после ошибки
void Synt::syncAfterError() {
    //Пропускаем проблемный токен
    if (currentTokenIndex < tokens.size()) {
        nextToken();
    }

    //Ищем точку восстановления
    while (currentTokenIndex < tokens.size()) {
        Token token = currentToken();

        if (token.getType() == TT_KEYWORD &&
            (token.getLexeme() == "INTEGER" || token.getLexeme() == "REAL" ||
                token.getLexeme() == "CALL" || token.getLexeme() == "END")) {
            return;
        }

        if (token.getType() == TT_IDENTIFIER) {
            //Проверяем не начало ли это оператора присваивания
            if (currentTokenIndex + 1 < tokens.size() &&
                tokens[currentTokenIndex + 1].getType() == TT_ASSIGN) {
                return;
            }
        }

        nextToken();
    }
}

//Восстановление до следующего оператора
void Synt::syncToNextOperator() {
    while (currentTokenIndex < tokens.size()) {
        Token token = currentToken();

        //Начало нового оператора или конструкции
        if (token.getType() == TT_IDENTIFIER) {
            //Проверяем не начало ли это оператора присваивания
            if (currentTokenIndex + 1 < tokens.size() &&
                tokens[currentTokenIndex + 1].getType() == TT_ASSIGN) {
                return;
            }
        }
        else if (token.getType() == TT_KEYWORD &&
            (token.getLexeme() == "CALL" || token.getLexeme() == "INTEGER" ||
                token.getLexeme() == "REAL" || token.getLexeme() == "END")) {
            return;
        }

        nextToken();
    }
}

//Восстановление до следующего выражения
void Synt::syncToEndOfExpression() {
    //Пропускаем до конца текущего выражениz
    while (currentTokenIndex < tokens.size()) {
        Token token = currentToken();

        //Конец выражения токены которые не могут быть частью выражения
        if (token.getType() == TT_KEYWORD &&
            (token.getLexeme() == "INTEGER" || token.getLexeme() == "REAL" ||
                token.getLexeme() == "CALL" || token.getLexeme() == "END")) {
            return;
        }

        //Идентификатор за которым следует '=' начало нового оператора
        if (token.getType() == TT_IDENTIFIER &&
            currentTokenIndex + 1 < tokens.size() &&
            tokens[currentTokenIndex + 1].getType() == TT_ASSIGN) {
            return;
        }

        nextToken();
    }
}

void Synt::syncToNextArgument() {
    //Пропускаем до следующего аргумента или конца списка
    while (currentTokenIndex < tokens.size()) {
        Token token = currentToken();

        if (token.getType() == TT_COMMA || token.getType() == TT_RPAREN) {
            return;
        }

        //Начало корректного выражения
        if (token.getType() == TT_IDENTIFIER || token.getType() == TT_INTEGER ||
            token.getType() == TT_REAL || token.getType() == TT_LPAREN) {
            return;
        }

        nextToken();
    }
}


void Synt::synt() {
    root = parseProgram();

    if (!errors.empty()) {
        for (const auto& errorMsg : errors) {
            astOutput << errorMsg << "\n";
        }
        astOutput << "Parsing completed with " << errors.size() << " error(s)\n";
    }
    else {
        astOutput << "Parsing completed successfully!\n";
    }
}

//Парсинг ветки Program
Node* Synt::parseProgram() {
    auto node = new Node("Program");
    printNode("Program");
    increaseIndent();

    auto beginNode = parseBegin();
    if (beginNode) {
        node->addChild(beginNode);
    }
    else {
        syncAfterError();
    }

    auto descriptionsNode = parseDescriptions();
    if (descriptionsNode) {
        node->addChild(descriptionsNode);
    }

    auto operatorsNode = parseOperators();
    if (operatorsNode) {
        node->addChild(operatorsNode);
    }

    auto endNode = parseEnd();
    if (endNode) {
        node->addChild(endNode);
    }
    else {
        error("Expected END statement");
    }

    //Проверяем что обработали все токены (кроме END)
    if (currentTokenIndex < tokens.size()) {
        Token unexpected = currentToken();
        std::stringstream errorMsg;
        errorMsg << "Unexpected token(s) '" << unexpected.getLexeme()
            << "' after END";
        error(errorMsg.str());
    }

    decreaseIndent();
    return node;
}

//Begin -> PROGRAM Id
Node* Synt::parseBegin() {
    auto node = new Node("Begin");
    printNode("Begin");
    increaseIndent();

    if (matchKeyword("PROGRAM")) {
        auto programNode = new Node("KEYWORD", "PROGRAM", currentToken().getLine());
        node->addChild(programNode);
        printLeaf("PROGRAM");

        if (match(TT_IDENTIFIER)) {
            auto idNode = new Node("IDENTIFIER", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
            node->addChild(idNode);
            printLeaf(tokens[currentTokenIndex - 1].getLexeme());
        }
        else {
            error("Expected identifier after PROGRAM");
            //Восстанавливаемся пропускаем до начала описаний или операторов
            while (currentTokenIndex < tokens.size()) {
                Token token = currentToken();
                if (token.getType() == TT_KEYWORD &&
                    (token.getLexeme() == "INTEGER" || token.getLexeme() == "REAL" ||
                        token.getLexeme() == "CALL" || token.getType() == TT_IDENTIFIER)) {
                    break;
                }
                nextToken();
            }
        }
    }
    else {
        error("Expected 'PROGRAM'");
        //Восстанавливаемся - ищем идентификатор программы
        while (currentTokenIndex < tokens.size()) {
            if (currentToken().getType() == TT_IDENTIFIER) {
                //Нашли возможный идентификатор программы
                auto idNode = new Node("IDENTIFIER", tokens[currentTokenIndex].getLexeme(), tokens[currentTokenIndex].getLine());
                node->addChild(idNode);
                printLeaf(tokens[currentTokenIndex].getLexeme());
                nextToken();
                break;
            }
            nextToken();
        }
    }

    decreaseIndent();
    return node;
}

//Парсинг ветки описаний
Node* Synt::parseDescriptions() {
    auto node = new Node("Descriptions");
    printNode("Descriptions");
    increaseIndent();

    //Обрабатываем все последовательные описания
    while (currentTokenIndex < tokens.size() &&
        currentToken().getType() == TT_KEYWORD &&
        (currentToken().getLexeme() == "INTEGER" || currentToken().getLexeme() == "REAL")) {
        auto descrNode = parseDescr();
        if (descrNode) {
            node->addChild(descrNode);
        }
    }

    decreaseIndent();
    return node;
}

//Descr -> Type VarList
Node* Synt::parseDescr() {
    auto node = new Node("Descr");
    printNode("Descr");
    increaseIndent();

    auto typeNode = parseType();
    if (typeNode) {
        node->addChild(typeNode);
    }

    auto varListNode = parseVarList();
    if (varListNode) {
        node->addChild(varListNode);
    }

    decreaseIndent();
    return node;
}

//Type -> INTEGER | REAL
Node* Synt::parseType() {
    auto node = new Node("Type");
    printNode("Type");
    increaseIndent();

    if (matchKeyword("INTEGER")) {
        auto intNode = new Node("KEYWORD", "INTEGER", tokens[currentTokenIndex - 1].getLine());
        node->addChild(intNode);
        printLeaf("INTEGER");
    }
    else if (matchKeyword("REAL")) {
        auto realNode = new Node("KEYWORD", "REAL", tokens[currentTokenIndex - 1].getLine());
        node->addChild(realNode);
        printLeaf("REAL");
    }
    else {
        error("Expected 'INTEGER' or 'REAL'");
    }

    decreaseIndent();
    return node;
}

//VarList -> Id | Id , VarList
Node* Synt::parseVarList() {
    auto node = new Node("VarList");
    printNode("VarList");
    increaseIndent();

    if (match(TT_IDENTIFIER)) {
        auto idNode = new Node("IDENTIFIER", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
        node->addChild(idNode);
        printLeaf(tokens[currentTokenIndex - 1].getLexeme());

        while (match(TT_COMMA)) {
            auto commaNode = new Node("COMMA", ",", tokens[currentTokenIndex - 1].getLine());
            node->addChild(commaNode);
            printLeaf(",");

            if (match(TT_IDENTIFIER)) {
                auto nextIdNode = new Node("IDENTIFIER", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
                node->addChild(nextIdNode);
                printLeaf(tokens[currentTokenIndex - 1].getLexeme());
            }
            else {
                error("Expected identifier after comma");
                break;
            }
        }
    }
    else {
        error("Expected identifier in variable list");
    }

    decreaseIndent();
    return node;
}

//Парсинг ветки операторов
Node* Synt::parseOperators() {
    auto node = new Node("Operators");
    printNode("Operators");
    increaseIndent();

    // Обрабатываем все последовательные операторы
    while (currentTokenIndex < tokens.size() &&
        !(currentToken().getType() == TT_KEYWORD && currentToken().getLexeme() == "END") &&
        (currentToken().getType() == TT_IDENTIFIER ||
            (currentToken().getType() == TT_KEYWORD && currentToken().getLexeme() == "CALL"))) {
        auto opNode = parseOp();
        if (opNode) {
            node->addChild(opNode);
        }
    }

    decreaseIndent();
    return node;
}

//Op -> Id = Expr | CALL Id ( VarList )
Node* Synt::parseOp() {
    auto node = new Node("Op");
    printNode("Op");
    increaseIndent();

    if (currentToken().getType() == TT_IDENTIFIER) {
        // Присваивание: Id = Expr
        std::string identifier = currentToken().getLexeme();
        auto idNode = new Node("IDENTIFIER", identifier, currentToken().getLine());
        node->addChild(idNode);
        nextToken();
        printLeaf(identifier);

        if (match(TT_ASSIGN)) {
            auto assignNode = new Node("ASSIGN", "=", tokens[currentTokenIndex - 1].getLine());
            node->addChild(assignNode);
            printLeaf("=");

            auto exprNode = parseExpr();
            if (exprNode) {
                node->addChild(exprNode);
            }
            else {
                //Если выражение содержит ошибки, восстанавливаемся
                syncToEndOfExpression();
            }

            // После разбора выражения проверяем, нет ли лишнего = 
            if (currentTokenIndex < tokens.size() &&
                currentToken().getType() == TT_ASSIGN) {
                error("Unexpected '=' after expression");
                syncToNextOperator();
            }
        }
        else {
            error("Expected '=' in assignment");
            syncToNextOperator();
        }
    }
    else if (currentToken().getType() == TT_KEYWORD && currentToken().getLexeme() == "CALL") {
        //CALL Id ( arguments )
        auto callNode = new Node("KEYWORD", "CALL", currentToken().getLine());
        node->addChild(callNode);
        printLeaf("CALL");
        nextToken();

        if (match(TT_IDENTIFIER)) {
            auto procNode = new Node("IDENTIFIER", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
            node->addChild(procNode);
            printLeaf(tokens[currentTokenIndex - 1].getLexeme());

            if (match(TT_LPAREN)) {
                auto lparenNode = new Node("LPAREN", "(", tokens[currentTokenIndex - 1].getLine());
                node->addChild(lparenNode);
                printLeaf("(");

                auto argsNode = parseCallArguments();
                if (argsNode) {
                    node->addChild(argsNode);
                }

                if (match(TT_RPAREN)) {
                    auto rparenNode = new Node("RPAREN", ")", tokens[currentTokenIndex - 1].getLine());
                    node->addChild(rparenNode);
                    printLeaf(")");
                }
                else {
                    error("Expected ')' after arguments");
                    syncToNextOperator();
                }
            }
            else {
                error("Expected '(' after CALL identifier");
                syncToNextOperator();
            }
        }
        else {
            error("Expected identifier after CALL");
            syncToNextOperator();
        }
    }

    decreaseIndent();
    return node;
}

Node* Synt::parseCallArguments() {
    auto node = new Node("Arguments");

    if (currentTokenIndex < tokens.size() &&
        currentToken().getType() != TT_RPAREN) {

        auto firstArg = parseExpr();
        if (firstArg) {
            node->addChild(firstArg);
        }
        else {
            syncToNextArgument();
        }

        while (match(TT_COMMA)) {
            auto commaNode = new Node("COMMA", ",", tokens[currentTokenIndex - 1].getLine());
            node->addChild(commaNode);
            printLeaf(",");

            auto nextArg = parseExpr();
            if (nextArg) {
                node->addChild(nextArg);
            }
            else {
                syncToNextArgument();
            }
        }
    }

    return node;
}

//Expr -> SimpleExpr | SimpleExpr + Expr | SimpleExpr - Expr
Node* Synt::parseExpr() {
    auto node = new Node("Expr");
    printNode("Expr");
    increaseIndent();

    auto leftNode = parseSimpleExpr();
    if (leftNode) {
        node->addChild(leftNode);
    }

    if (currentTokenIndex < tokens.size()) {
        TokenType opType = currentToken().getType();
        while (opType == TT_PLUS || opType == TT_MINUS) {
            if(opType == TT_PLUS){
                auto opNode = new Node("PLUS", "+", currentToken().getLine());
                printLeaf("+");
                node->addChild(opNode);
            }
            else{
                auto opNode = new Node("MINUS", "-", currentToken().getLine());
                printLeaf("-");
                node->addChild(opNode);
            }
           
            nextToken();
            

            auto rightNode = parseSimpleExpr();
            if (rightNode) {
                node->addChild(rightNode);
            }
            else {
                break;
            }

            if (currentTokenIndex >= tokens.size()) break;
            opType = currentToken().getType();
        }
    }

    decreaseIndent();
    return node;
}

//SimpleExpr -> Id | Const | ( Expr )
Node* Synt::parseSimpleExpr() {
    auto node = new Node("SimpleExpr");
    printNode("SimpleExpr");
    increaseIndent();

    if (currentTokenIndex >= tokens.size()) {
        error("Unexpected end of input in expression");
    }
    else if (match(TT_IDENTIFIER)) {
        auto idNode = new Node("IDENTIFIER", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
        node->addChild(idNode);
        printLeaf(tokens[currentTokenIndex - 1].getLexeme());
    }
    else if (match(TT_INTEGER)) {
        auto intNode = new Node("INTEGER", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
        node->addChild(intNode);
        printLeaf(tokens[currentTokenIndex - 1].getLexeme());
    }
    else if (match(TT_REAL)) {
        auto realNode = new Node("REAL", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
        node->addChild(realNode);
        printLeaf(tokens[currentTokenIndex - 1].getLexeme());
    }
    else if (match(TT_LPAREN)) {
        auto lparenNode = new Node("LPAREN", "(", tokens[currentTokenIndex - 1].getLine());
        node->addChild(lparenNode);
        printLeaf("(");

        auto exprNode = parseExpr();
        if (exprNode) {
            node->addChild(exprNode);
        }

        if (match(TT_RPAREN)) {
            auto rparenNode = new Node("RPAREN", ")", tokens[currentTokenIndex - 1].getLine());
            node->addChild(rparenNode);
            printLeaf(")");
        }
        else {
            error("Expected ')' after expression");
            syncToEndOfExpression();
        }
    }
    else {
        Token token = currentToken();
        if (token.getType() == TT_PLUS || token.getType() == TT_MINUS ||
            token.getType() == TT_RPAREN || token.getType() == TT_COMMA) {
            error("Missing operand in expression");
        }
        else {
            error("Expected identifier, constant, or '('");
        }
        syncToEndOfExpression();
    }

    decreaseIndent();
    return node;
}

Node* Synt::parseEnd() {
    auto node = new Node("End");
    printNode("End");
    increaseIndent();

    if (matchKeyword("END")) {
        auto endNode = new Node("KEYWORD", "END", tokens[currentTokenIndex - 1].getLine());
        node->addChild(endNode);
        printLeaf("END");

        if (match(TT_IDENTIFIER)) {
            auto idNode = new Node("IDENTIFIER", tokens[currentTokenIndex - 1].getLexeme(), tokens[currentTokenIndex - 1].getLine());
            node->addChild(idNode);
            printLeaf(tokens[currentTokenIndex - 1].getLexeme());
        }
        else {
            error("Expected identifier after END");
        }
    }
    else {
        error("Expected 'END'");
    }

    decreaseIndent();
    return node;
}