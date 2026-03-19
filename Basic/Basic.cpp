/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::string msg = ex.getMessage();
            if (msg == "QUIT") {
                break; // Exit the loop normally
            }
            std::cout << msg << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) {
        return; // Empty line
    }

    std::string firstToken = scanner.nextToken();

    // Check if the line starts with a number (program line)
    bool isNumber = true;
    for (char c : firstToken) {
        if (!isdigit(c)) {
            isNumber = false;
            break;
        }
    }

    if (isNumber) {
        // Program line: <line number> <statement>
        int lineNumber = stringToInteger(firstToken);

        if (!scanner.hasMoreTokens()) {
            // Empty line after line number: delete the line
            program.removeSourceLine(lineNumber);
            return;
        }

        // Store the source line
        program.addSourceLine(lineNumber, line);

        // Parse and store the statement (will be parsed when RUN is called)
        // For now, we don't parse it immediately
        return;
    }

    // Immediate command
    std::string command = firstToken;

    if (command == "QUIT") {
        throw ErrorException("QUIT");
    } else if (command == "HELP") {
        std::cout << "Yet another basic interpreter" << std::endl;
    } else if (command == "LIST") {
        int lineNum = program.getFirstLineNumber();
        while (lineNum != -1) {
            std::cout << program.getSourceLine(lineNum) << std::endl;
            lineNum = program.getNextLineNumber(lineNum);
        }
    } else if (command == "CLEAR") {
        program.clear();
        state.Clear();
    } else if (command == "RUN") {
        // Execute the program
        int lineNum = program.getFirstLineNumber();
        while (lineNum != -1) {
            Statement* stmt = program.getParsedStatement(lineNum);
            if (stmt == nullptr) {
                // Parse the statement
                std::string sourceLine = program.getSourceLine(lineNum);
                // Remove line number prefix
                size_t spacePos = sourceLine.find(' ');
                if (spacePos != std::string::npos) {
                    std::string statementStr = sourceLine.substr(spacePos + 1);
                    TokenScanner stmtScanner;
                    stmtScanner.ignoreWhitespace();
                    stmtScanner.scanNumbers();
                    stmtScanner.setInput(statementStr);

                    std::string stmtType = stmtScanner.nextToken();
                    if (stmtType == "REM") {
                        stmt = new RemStatement();
                    } else if (stmtType == "LET") {
                        std::string var = stmtScanner.nextToken();
                        stmtScanner.verifyToken("=");
                        Expression* exp = parseExp(stmtScanner);
                        stmt = new LetStatement(exp, var);
                    } else if (stmtType == "PRINT") {
                        Expression* exp = parseExp(stmtScanner);
                        stmt = new PrintStatement(exp);
                    } else if (stmtType == "INPUT") {
                        std::string var = stmtScanner.nextToken();
                        stmt = new InputStatement(var);
                    } else if (stmtType == "END") {
                        stmt = new EndStatement();
                    } else if (stmtType == "GOTO") {
                        std::string lineNumStr = stmtScanner.nextToken();
                        int targetLine = stringToInteger(lineNumStr);
                        stmt = new GotoStatement(targetLine);
                    } else if (stmtType == "IF") {
                        // Parse IF statement: IF <exp1> <op> <exp2> THEN <line>
                        Expression* lhs = parseExp(stmtScanner);
                        std::string op = stmtScanner.nextToken();
                        Expression* rhs = parseExp(stmtScanner);
                        stmtScanner.verifyToken("THEN");
                        std::string targetLineStr = stmtScanner.nextToken();
                        int targetLine = stringToInteger(targetLineStr);
                        stmt = new IfStatement(lhs, op, rhs, targetLine);
                    } else {
                        error("SYNTAX ERROR");
                    }
                    program.setParsedStatement(lineNum, stmt);
                }
            }

            // Execute the statement
            try {
                stmt->execute(state, program);
                lineNum = program.getNextLineNumber(lineNum);
            } catch (ErrorException &ex) {
                std::string msg = ex.getMessage();
                if (msg.find("GOTO") == 0) {
                    // Extract line number from "GOTO <line>"
                    size_t spacePos = msg.find(' ');
                    int targetLine = stringToInteger(msg.substr(spacePos + 1));
                    lineNum = targetLine;
                    // Check if target line exists
                    if (program.getSourceLine(lineNum).empty()) {
                        error("LINE NUMBER ERROR");
                    }
                } else if (msg == "END") {
                    return; // End of program
                } else {
                    throw; // Re-throw other errors
                }
            }
        }
    } else if (command == "REM") {
        // REM comment in immediate mode - do nothing
    } else if (command == "LET") {
        // Immediate LET
        std::string var = scanner.nextToken();
        scanner.verifyToken("=");
        Expression* exp = parseExp(scanner);
        LetStatement stmt(exp, var);
        stmt.execute(state, program);
        // exp is deleted by LetStatement destructor
    } else if (command == "PRINT") {
        // Immediate PRINT
        Expression* exp = parseExp(scanner);
        PrintStatement stmt(exp);
        stmt.execute(state, program);
        // exp is deleted by PrintStatement destructor
    } else if (command == "INPUT") {
        // Immediate INPUT
        std::string var = scanner.nextToken();
        InputStatement stmt(var);
        stmt.execute(state, program);
    } else if (command == "END") {
        // Immediate END
        EndStatement stmt;
        stmt.execute(state, program);
    } else if (command == "GOTO") {
        error("GOTO CANNOT BE USED IN IMMEDIATE MODE");
    } else if (command == "IF") {
        error("IF CANNOT BE USED IN IMMEDIATE MODE");
    } else {
        error("SYNTAX ERROR");
    }
}

