#pragma once
#include "Token.h"
#include <vector>
#include <fstream>
#include <string>

struct Node {
    std::string name;
    std::string value;
    std::vector<Node*> children;
    int lineNumber;

    Node(const std::string& nodeName, const std::string& nodeValue = "", int line = -1)
        : name(nodeName), value(nodeValue), lineNumber(line) {
    }

    void addChild(Node* child) {
        children.push_back(child);
    }
};

class Synt {
private:
    std::vector<Token> tokens;
    size_t currentTokenIndex;
    std::ofstream& astOutput;
    int indentLevel;
    int lineNumber;
    std::vector<std::string> errors;
    bool inDescriptionsSection;

    //Корень дерева разбора
    Node* root;

    //Вспомогательные методы
    Token currentToken() const;
    void nextToken();
    bool match(TokenType expected);
    bool matchKeyword(const std::string& keyword);

    void error(const std::string& message);
    void syncAfterError();
    void syncToNextOperator();
    void syncToNextArgument();
    void syncToEndOfExpression();

    //Методы для форматирования и вывода дерева разбора
    void printNode(const std::string& nodeName);
    void printLeaf(const std::string& value);
    void printNumberedLine(const std::string& line);
    void increaseIndent();
    void decreaseIndent();

    //Методы для работы с грамматикой
    Node* parseProgram();
    Node* parseBegin();
    Node* parseEnd();
    Node* parseDescriptions();
    Node* parseDescr();
    Node* parseType();
    Node* parseVarList();
    Node* parseOperators();
    Node* parseOp();
    Node* parseExpr();
    Node* parseSimpleExpr();
    Node* parseCallArguments();

public:
    Synt(const std::vector<Token>& tokenList, std::ofstream& output);
    void synt();
    Node* getTree() const { return root; }
};