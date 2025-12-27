#pragma once
#include "Token.h"
#include "Synt.h"
#include "HashTable.h"
#include <vector>
#include <string>
#include <stack>
#include <fstream>
#include <iostream>
#include <algorithm>

struct VarInfo {
    std::string name;
    TokenType type;
    bool initialized;

    VarInfo() : name(""), type(TT_UNKNOWN), initialized(false) {}
    VarInfo(const std::string& n, TokenType t, bool init = false)
        : name(n), type(t), initialized(init) {
    }

    std::string getKey() const { return name; }

    std::string typeToString() const {
        switch (type) {
        case TT_INTEGER: return "INTEGER";
        case TT_REAL: return "REAL";
        default: return "UNKNOWN";
        }
    }
};

class SemanticAnalyzer {
private:
    Node* astRoot;
    std::ofstream& output;
    std::vector<std::string> errors;
    std::vector<std::string> postfixCode;

    HashTable<VarInfo> varTable;

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

    //Метод для выражений
    TokenType analyzeExpression(Node* exprNode, std::string& postfix);

    //Проверки
    void checkVariableDeclared(const std::string& varName, int line);
    void checkVariableNotRedeclared(const std::string& varName, int line);
    void checkTypeCompatibility(TokenType leftType, TokenType rightType, int line);
    void checkProgramNameMatch(const std::string& endName, int line);
    bool checkExpressionTypes(Node* node, bool& hasReal, bool& hasInteger);

    //Постфиксная запись
    void expressionToPostfix(Node* exprNode, std::string& result);

    VarInfo* findVar(const std::string& name);

    //Вспомогательные методы для обхода дерева
    void RealCheck(Node* node, bool& hasReal);
    void convertToPostfix(Node* node, std::vector<std::string>& output,
        std::stack<std::string>& operators);

public:
    SemanticAnalyzer(Node* root, std::ofstream& outputStream);
    void analyze();
    std::vector<std::string> getErrors() const { return errors; }
};