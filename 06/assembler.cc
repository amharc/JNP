#include "assembler.h"

#include <cctype>
#include <limits>
#include <sstream>

namespace computer_internal {
Assembler::Assembler() { }

void Assembler::Parser::require(bool condition, const std::string &why) const {
    if(!condition)
        throw ParserException(why, line, pos);
}

void Assembler::Parser::removeTrailingDOSEndline() {
    if(!line.empty() && line.back() == '\r')
        line.pop_back();
}

Assembler::Parser::Parser(const std::string &line) : line{line}, pos{0} { 
    removeTrailingDOSEndline();
}

Assembler::Parser::Parser(std::string &&line) : line{std::move(line)}, pos{0} {
    removeTrailingDOSEndline();
}

bool Assembler::Parser::hasNext() const {
    return line.size() > pos;
}

char Assembler::Parser::getNext() {
    require(hasNext(), "Unexpected end of line");
    return line[pos++];
}

void Assembler::Parser::skipSpaces() {
    while(pos < line.size() && std::isspace(line[pos]))
        ++pos;
}

template<class T>
T Assembler::Parser::parseIntegral() {
    static_assert(std::numeric_limits<T>::max()
        < std::numeric_limits<uintmax_t>::max() / 10,
        "Unable to parse numbers that big");

    uintmax_t res = 0;
    bool negative = false;
    int len;
    for(len = 0; res < std::numeric_limits<T>::max() && hasNext(); ++len) {
        char c = getNext();
        if(std::isspace(c))
            break;

        if(c == '-') {
            require(len == 0, "Minus sign should not appear inside a number");
            require(std::numeric_limits<T>::is_signed, "No sign expected");
            negative = true;
            continue;
        }

        require(isdigit(c), std::string{"Expected a digit, got "} + c);
        res = res * 10 + (c - '0');
    }

    if(negative) {
        require(len > 1, "Expected a number");
        // Check for overflow (assuming two's complement)
        require(res + 1 < std::numeric_limits<T>::max(), "Number is too big");
        return -static_cast<T>(res);
    }
    else {
        require(len > 0, "Expected a number");
        require(res < std::numeric_limits<T>::max(), "Number is too big");
        return static_cast<T>(res);
    }
}

register_type Assembler::Parser::parseRegister() {
    skipSpaces();
    require(getNext() == 'R', "Expected a register");
    return parseIntegral<register_type>();
}

memory_type Assembler::Parser::parseAddress() {
    skipSpaces();
    require(getNext() == 'M', "Expected a memory address");
    return parseIntegral<memory_type>();
}

number_type Assembler::Parser::parseNumber() {
    skipSpaces();
    return parseIntegral<number_type>();
}

void Assembler::Parser::end() {
    skipSpaces();
    require(pos == line.size(), "Trailing characters");
}

std::string Assembler::Parser::getWord() {
    // Type disambiguation wrapper
    skipSpaces();
    auto isspace = [](char c) { return std::isspace(c); };
    auto it = std::find_if(line.begin() + pos, line.end(), isspace);
    auto next_space = it - line.begin();

    std::string result = line.substr(pos, next_space - pos);
    pos = next_space;
    return result;
}

std::shared_ptr<const Instruction> Assembler::compileLine(std::string &&line) {
    Parser parser{std::move(line)};
    std::string op = parser.getWord();
    if(op == "SET") {
        auto reg = parser.parseRegister();
        auto val = parser.parseNumber();
        parser.end();
        return std::make_shared<const SetInstruction>(reg, val);
    }
    else if(op == "LOAD") {
        auto reg = parser.parseRegister();
        auto mem = parser.parseAddress();
        parser.end();
        return std::make_shared<const LoadInstruction>(reg, mem);
    }
    else if(op == "STORE") {
        auto mem = parser.parseAddress();
        auto reg = parser.parseRegister();
        parser.end();
        return std::make_shared<const StoreInstruction>(mem, reg);
    }
    else if(op == "ADD") {
        auto lhs = parser.parseRegister();
        auto rhs = parser.parseRegister();
        parser.end();
        return std::make_shared<const AddInstruction>(lhs, rhs);
    }
    else if(op == "SUB") {
        auto lhs = parser.parseRegister();
        auto rhs = parser.parseRegister();
        parser.end();
        return std::make_shared<const SubInstruction>(lhs, rhs);
    }
    else if(op == "MUL") {
        auto lhs = parser.parseRegister();
        auto rhs = parser.parseRegister();
        parser.end();
        return std::make_shared<const MulInstruction>(lhs, rhs);
    }
    else if(op == "DIV") {
        auto lhs = parser.parseRegister();
        auto rhs = parser.parseRegister();
        parser.end();
        return std::make_shared<const DivInstruction>(lhs, rhs);
    }
    else if(op == "PRINTLN") {
        auto reg = parser.parseRegister();
        parser.end();
        return std::make_shared<const PrintlnInstruction>(reg);
    }
    else if(op == "") { // empty line (except for whitespace)
        return {};
    }
    else
        throw UnknownInstructionException(op);
}

std::shared_ptr<Program> Assembler::compile(const std::string &code) {
    std::istringstream stream{code};
    std::string line;
    auto compiled = std::make_shared<Program>();

    while(std::getline(stream, line)) {
        auto instruction = compileLine(std::move(line));
        if(instruction)
            compiled->push_back(instruction);
    }

    return compiled;
}
} // namespace computer_internal
