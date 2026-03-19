/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"


/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// REM statement implementation
RemStatement::RemStatement() = default;

RemStatement::~RemStatement() = default;

void RemStatement::execute(EvalState &state, Program &program) {
    // REM statements do nothing
}

// LET statement implementation
LetStatement::LetStatement(Expression *exp, std::string var) : exp(exp), var(var) {}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    state.setValue(var, value);
}

// PRINT statement implementation
PrintStatement::PrintStatement(Expression *exp) : exp(exp) {}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    std::cout << value << std::endl;
}

// INPUT statement implementation
InputStatement::InputStatement(std::string var) : var(var) {}

InputStatement::~InputStatement() = default;

void InputStatement::execute(EvalState &state, Program &program) {
    while (true) {
        std::cout << " ? " << std::flush;
        std::string input;
        if (!getline(std::cin, input)) {
            // EOF
            break;
        }

        // Check if input is a valid integer
        bool valid = true;
        for (size_t i = 0; i < input.size(); i++) {
            if (i == 0 && input[i] == '-') {
                // Allow negative sign at beginning
                continue;
            }
            if (!isdigit(input[i])) {
                valid = false;
                break;
            }
        }

        if (valid && !input.empty()) {
            try {
                int value = stringToInteger(input);
                state.setValue(var, value);
                break;
            } catch (ErrorException &) {
                // Invalid integer format
                valid = false;
            }
        }

        if (!valid) {
            // Reprompt (BASIC typically shows "?" or similar)
            continue;
        }
    }
}

// END statement implementation
EndStatement::EndStatement() = default;

EndStatement::~EndStatement() = default;

void EndStatement::execute(EvalState &state, Program &program) {
    // End statement should terminate program execution
    // We'll handle this in the program execution loop
    throw ErrorException("END");
}

// GOTO statement implementation
GotoStatement::GotoStatement(int lineNumber) : lineNumber(lineNumber) {}

GotoStatement::~GotoStatement() = default;

void GotoStatement::execute(EvalState &state, Program &program) {
    // GOTO should jump to the specified line
    // We'll handle this in the program execution loop
    throw ErrorException("GOTO " + integerToString(lineNumber));
}

// IF statement implementation
IfStatement::IfStatement(Expression *lhs, std::string op, Expression *rhs, int lineNumber)
    : lhs(lhs), op(op), rhs(rhs), lineNumber(lineNumber) {}

IfStatement::~IfStatement() {
    delete lhs;
    delete rhs;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int leftVal = lhs->eval(state);
    int rightVal = rhs->eval(state);
    bool condition = false;

    if (op == "=") {
        condition = (leftVal == rightVal);
    } else if (op == "<") {
        condition = (leftVal < rightVal);
    } else if (op == ">") {
        condition = (leftVal > rightVal);
    } else {
        error("Unknown comparison operator: " + op);
    }

    if (condition) {
        // Jump to the specified line
        throw ErrorException("GOTO " + integerToString(lineNumber));
    }
}
