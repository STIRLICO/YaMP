#pragma once
#include "Token.h"
#include "Synt.h"
#include <vector>
#include <string>
#include <set>
#include <stack>
#include <fstream>
#include <iostream>
#include <algorithm>

struct VarInfo {
    std::string name;
    TokenType type;
    bool initialized;
};

struct FunctionInfo {
    std::string name;
    std::vector<TokenType> parameterTypes;
};

class SemanticAnalyzer {
private:
    Node* astRoot;
    std::ofstream& output;
    std::vector<std::string> errors;
    std::vector<std::string> postfixCode;

    //Таблицы символов
    std::vector<VarInfo> Variables;
    std::vector<FunctionInfo> functions;
    std::set<std::string> declaredVariables;

    std::string programName;

    //Основные методы анализа
    void processDescriptionsNode(Node* node);
    void processOperatorsNode(Node* node);
    void processEndNode(Node* node);
    void processOpNode(Node* node);

    //Обработка конкретных узлов
    void processBegin(Node* node);
    void processVarList(Node* node, TokenType type);
    void processAssignment(Node* idNode, Node* exprNode);
    void processCall(Node* callNode);
    void processEnd(Node* node);

    //Методы для выражений
    TokenType analyzeExpression(Node* exprNode, std::string& postfix);

    //Проверки
    void checkVariableDeclared(const std::string& varName, int line);
    void checkVariableNotRedeclared(const std::string& varName, int line);
    void checkTypeCompatibility(TokenType leftType, TokenType rightType, int line);
    void checkProgramNameMatch(const std::string& endName, int line);

    //Постфиксная запись
    void expressionToPostfix(Node* exprNode, std::string& result);

    //Утилиты для работы с наборами
    VarInfo* findVar(const std::string& name);

    //Вспомогательные методы для обхода дерева
    void RealCheck(Node* node, bool& hasReal);
    void convertToPostfix(Node* node, std::vector<std::string>& output, std::stack<std::string>& operators);

public:
    SemanticAnalyzer(Node* root, std::ofstream& outputStream);
    void analyze();
    std::vector<std::string> getErrors() const { return errors; }
};