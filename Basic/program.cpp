/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"



Program::Program() {
    // Initialize empty program
}

Program::~Program() {
    clear(); // Free all allocated statements
}

void Program::clear() {
    // Free all allocated statements
    for (auto& entry : lines) {
        if (entry.second.second != nullptr) {
            delete entry.second.second;
        }
    }
    lines.clear();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    auto it = lines.find(lineNumber);
    if (it != lines.end()) {
        // Line exists, free the old parsed statement
        if (it->second.second != nullptr) {
            delete it->second.second;
        }
        it->second.first = line;
        it->second.second = nullptr;
    } else {
        // Insert new line in sorted order (map automatically sorts by key)
        lines[lineNumber] = std::make_pair(line, nullptr);
    }
}

void Program::removeSourceLine(int lineNumber) {
    auto it = lines.find(lineNumber);
    if (it != lines.end()) {
        if (it->second.second != nullptr) {
            delete it->second.second;
        }
        lines.erase(it);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = lines.find(lineNumber);
    if (it != lines.end()) {
        return it->second.first;
    }
    return "";
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    auto it = lines.find(lineNumber);
    if (it == lines.end()) {
        error("setParsedStatement: Line " + integerToString(lineNumber) + " not found");
    }
    // Free old statement if exists
    if (it->second.second != nullptr) {
        delete it->second.second;
    }
    it->second.second = stmt;
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
    auto it = lines.find(lineNumber);
    if (it != lines.end()) {
        return it->second.second;
    }
    return nullptr;
}

int Program::getFirstLineNumber() {
    if (lines.empty()) {
        return -1;
    }
    return lines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = lines.find(lineNumber);
    if (it == lines.end()) {
        error("getNextLineNumber: Line " + integerToString(lineNumber) + " not found");
    }
    ++it;
    if (it == lines.end()) {
        return -1;
    }
    return it->first;
}

//more func to add
//todo


