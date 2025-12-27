#include "Semantic.h"
#include <sstream>
#include <iomanip>
#include <stack>
#include <algorithm>

SemanticAnalyzer::SemanticAnalyzer(Node* root, std::ofstream& outputStream)
    : astRoot(root), output(outputStream), programName(""), varTable(211) {
}

VarInfo* SemanticAnalyzer::findVar(const std::string& name) {
    int idx = varTable.findIndex(name);
    if (idx != -1) {
        return varTable.getValue(idx);
    }
    return nullptr;
}

void SemanticAnalyzer::analyze() {
    if (!astRoot) {
        output << "ERROR: AST is empty\n";
        return;
    }

    processDescriptionsNode(astRoot);
    processOperatorsNode(astRoot);
    processEndNode(astRoot);

    if (!errors.empty()) {
        output << "\nERRORS:\n";
        for (const auto& error : errors) {
            output << error << std::endl;
        }
        output << "\nSemantic analysis completed with " << errors.size() << " error(s)";
    }
    else {
        output << "\nSemantic analysis completed successfully!";
    }
}

void SemanticAnalyzer::processDescriptionsNode(Node* node) {
    if (!node) return;

    if (node->name == "Begin") {
        processBegin(node);
    }
    else if (node->name == "Descriptions") {
        //Обрабатываем описания
        TokenType currentType = TT_UNKNOWN;

        for (auto child : node->children) {
            if (child->name == "Descr") {
                //Находим тип в потомках Descr
                for (auto descrChild : child->children) {
                    if (descrChild->name == "Type") {
                        for (auto typeChild : descrChild->children) {
                            if (typeChild->value == "INTEGER") {
                                currentType = TT_INTEGER;
                            }
                            else if (typeChild->value == "REAL") {
                                currentType = TT_REAL;
                            }
                        }
                    }
                    else if (descrChild->name == "VarList") {
                        processVarList(descrChild, currentType);
                    }
                }
            }
        }
    }
    //Рекурсивно обрабатываем детей
    for (auto child : node->children) {
        processDescriptionsNode(child);
    }
}

void SemanticAnalyzer::processOperatorsNode(Node* node) {
    if (!node) return;

    if (node->name == "Operators") {
        //Обрабатываем операторы последовательно
        for (auto child : node->children) {
            if (child->name == "Op") {
                processOpNode(child);
            }
        }
    }
    for (auto child : node->children) {
        processOperatorsNode(child);
    }
}

void SemanticAnalyzer::processEndNode(Node* node) {
    if (!node) return;

    if (node->name == "End") {
        processEnd(node);
    }
    for (auto child : node->children) {
        processEndNode(child);
    }
}

void SemanticAnalyzer::processOpNode(Node* node) {
    //Проверяем это присваивание или CALL
    bool isAssignment = false;
    bool isCall = false;
    Node* idNode = nullptr;
    Node* exprNode = nullptr;

    for (auto child : node->children) {
        if (child->name == "IDENTIFIER" && !isCall) {
            isAssignment = true;
            idNode = child;
        }
        else if (child->name == "KEYWORD" && child->value == "CALL") {
            isCall = true;
        }
        else if (child->name == "Expr") {
            exprNode = child;
        }
    }

    if (isAssignment && idNode && exprNode) {
        processAssignment(idNode, exprNode);
    }
    else if (isCall) {
        processCall(node);
    }
}

void SemanticAnalyzer::checkVariableDeclared(const std::string& varName, int line) {
    VarInfo* var = findVar(varName);
    if (!var) {
        std::stringstream ss;
        ss << "SEMANTIC ERROR at line " << line << ": Variable '" << varName << "' is not declared";
        errors.push_back(ss.str());
    }
}

void SemanticAnalyzer::checkVariableNotRedeclared(const std::string& varName, int line) {
    VarInfo* var = findVar(varName);
    if (var) {
        std::stringstream ss;
        ss << "SEMANTIC ERROR at line " << line << ": Variable '" << varName << "' is already declared";
        errors.push_back(ss.str());
    }
}

void SemanticAnalyzer::checkTypeCompatibility(TokenType leftType, TokenType rightType, int line) {
    if (leftType != rightType) {
        std::stringstream ss;
        ss << "SEMANTIC ERROR at line " << line << ": Type mismatch. Cannot assign ";

        if (rightType == TT_INTEGER) ss << "INTEGER";
        else if (rightType == TT_REAL) ss << "REAL";

        ss << " to ";

        if (leftType == TT_INTEGER) ss << "INTEGER";
        else if (leftType == TT_REAL) ss << "REAL";

        ss << " variable";
        errors.push_back(ss.str());
    }
}

void SemanticAnalyzer::checkProgramNameMatch(const std::string& endName, int line) {
    if (programName != endName) {
        std::stringstream ss;
        ss << "SEMANTIC ERROR at line " << line
            << ": Program name mismatch. Expected '" << programName
            << "', got '" << endName << "'";
        errors.push_back(ss.str());
    }
}

void SemanticAnalyzer::processBegin(Node* node) {
    for (auto child : node->children) {
        if (child->name == "IDENTIFIER") {
            programName = child->value;
            std::string Postfix = programName + " PROGRAM";
            postfixCode.push_back(Postfix);
            output << Postfix << "\n";
        }
    }
}

void SemanticAnalyzer::processVarList(Node* node, TokenType type) {
    std::vector<std::string> varNames;

    //Собираем все идентификаторы из VarList
    for (auto child : node->children) {
        if (child->name == "IDENTIFIER") {
            std::string varName = child->value;
            int line = child->lineNumber;

            checkVariableNotRedeclared(varName, line);

            //Если переменная не была объявлена ранее
            VarInfo* existingVar = findVar(varName);
            if (!existingVar) {
                varNames.push_back(varName);
                //Добавляем переменную в хэш-таблицу
                VarInfo info(varName, type, false);
                varTable.insert(varName, info);
            }
        }
    }

    //Если есть переменные для объявления
    if (!varNames.empty()) {
        std::string typeStr = (type == TT_INTEGER ? "INTEGER" : "REAL");
        std::string declPostfix = typeStr;

        //Добавляем имена переменных
        for (const auto& varName : varNames) {
            declPostfix += " " + varName;
        }
        declPostfix += " " + std::to_string(varNames.size() + 1) + " DECL";
        postfixCode.push_back(declPostfix);
        output << declPostfix << "\n";
    }
}

void SemanticAnalyzer::processAssignment(Node* idNode, Node* exprNode) {
    std::string varName = idNode->value;
    int line = idNode->lineNumber;

    //Проверка объявления переменной
    checkVariableDeclared(varName, line);

    // Анализ типа выражения
    std::string postfix;
    TokenType exprType = analyzeExpression(exprNode, postfix);

    //Проверка совместимости типов
    VarInfo* variable = findVar(varName);
    if (variable) {
        checkTypeCompatibility(variable->type, exprType, line);
        variable->initialized = true;
        std::string assignmentPostfix = varName + " " + postfix + " =";
        postfixCode.push_back(assignmentPostfix);
        output << assignmentPostfix << "\n";
    }
}

void SemanticAnalyzer::processCall(Node* node) {
    std::string funcName;
    std::vector<Node*> arguments;

    //Извлекаем имя функции и аргументы
    for (auto child : node->children) {
        if (child->name == "IDENTIFIER" && funcName.empty()) {
            funcName = child->value;
        }
        else if (child->name == "Arguments") {
            for (auto argChild : child->children) {
                if (argChild->name == "Expr") {
                    arguments.push_back(argChild);
                }
            }
        }
    }

    std::string callPostfix = funcName + " ";
    for (auto arg : arguments) {
        std::string argPostfix;
        //Проверка объявления переменных
        analyzeExpression(arg, argPostfix);
        callPostfix += argPostfix + " ";
    }
    callPostfix += std::to_string(arguments.size() + 1) + " CALL";
    postfixCode.push_back(callPostfix);
    output << callPostfix << "\n";
}

TokenType SemanticAnalyzer::analyzeExpression(Node* exprNode, std::string& postfix) {
    if (!exprNode) return TT_UNKNOWN;

    //Собираем постфиксную запись выражения
    expressionToPostfix(exprNode, postfix);

    //Определяем тип выражения и проверяем совместимость типов внутри него
    bool hasReal = false;
    bool hasInteger = false;
    bool isMixed = checkExpressionTypes(exprNode, hasReal, hasInteger);

    if (isMixed) {
        return TT_UNKNOWN;
    }

    return hasReal ? TT_REAL : TT_INTEGER;
}
bool SemanticAnalyzer::checkExpressionTypes(Node* node, bool& hasReal, bool& hasInteger) {
    if (!node) return false;

    if (node->name == "REAL") {
        hasReal = true;
    }
    else if (node->name == "INTEGER") {
        hasInteger = true;
    }
    else if (node->name == "IDENTIFIER") {
        VarInfo* symbol = findVar(node->value);
        if (symbol) {
            if (symbol->type == TT_REAL) {
                hasReal = true;
            }
            else if (symbol->type == TT_INTEGER) {
                hasInteger = true;
            }
        }
    }

    for (auto child : node->children) {
        checkExpressionTypes(child, hasReal, hasInteger);
    }

    return hasReal && hasInteger;
}

//Проверка наличия вещественных чисел
void SemanticAnalyzer::RealCheck(Node* node, bool& hasReal) {
    if (!node) return;

    if (node->name == "REAL") {
        hasReal = true;
    }
    else if (node->name == "IDENTIFIER") {
        VarInfo* symbol = findVar(node->value);
        if (symbol && symbol->type == TT_REAL) {
            hasReal = true;
        }
    }

    for (auto child : node->children) {
        RealCheck(child, hasReal);
    }
}

//Функция для обхода дерева и построения постфиксной записи
void SemanticAnalyzer::convertToPostfix(Node* node, std::vector<std::string>& outputVec,
    std::stack<std::string>& operators) {
    if (!node) return;

    if (node->name == "IDENTIFIER") {
        checkVariableDeclared(node->value, node->lineNumber);
        outputVec.push_back(node->value);
    }
    else if (node->name == "INTEGER" || node->name == "REAL") {
        outputVec.push_back(node->value);
    }
    else if (node->name == "PLUS") {
        std::string op = "+";
        operators.push(op);
    }
    else if (node->name == "MINUS") {
        std::string op = "-";
        operators.push(op);
    }
    else if (node->name == "LPAREN") {
        operators.push("(");
    }
    else if (node->name == "RPAREN") {
        while (!operators.empty() && operators.top() != "(") {
            outputVec.push_back(operators.top());
            operators.pop();
        }
        if (!operators.empty()) operators.pop(); //Удаляем (
    }

    for (auto child : node->children) {
        convertToPostfix(child, outputVec, operators);
    }
}

void SemanticAnalyzer::expressionToPostfix(Node* exprNode, std::string& result) {
    if (!exprNode) return;

    std::vector<std::string> outputVec;
    std::stack<std::string> operators;

    convertToPostfix(exprNode, outputVec, operators);
    while (!operators.empty()) {
        outputVec.push_back(operators.top());
        operators.pop();
    }

    for (size_t i = 0; i < outputVec.size(); i++) {
        if (i > 0) result += " ";
        result += outputVec[i];
    }
}

void SemanticAnalyzer::processEnd(Node* node) {
    std::string endName;
    int line = -1;

    for (auto child : node->children) {
        if (child->name == "IDENTIFIER") {
            endName = child->value;
            line = child->lineNumber;
        }
    }

    if (!endName.empty()) {
        checkProgramNameMatch(endName, line);
        std::string Postfix = endName + " END";
        postfixCode.push_back(Postfix);
        output << Postfix << "\n";
    }
}