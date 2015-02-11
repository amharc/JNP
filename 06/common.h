#ifndef _COMMON_H
#define _COMMON_H

#include <cstdint>
#include <cinttypes>
#include <string>
#include <stdexcept>

using number_type = int32_t;
using register_type = number_type;
using memory_type = number_type;
using time_type = number_type;

namespace computer_internal {
using long_number_type = int64_t;
}

class IllegalArgumentException : public std::invalid_argument {
    public:
    IllegalArgumentException()
        : std::invalid_argument("Illegal argument") { }
    using std::invalid_argument::invalid_argument;
};

class UnknownInstructionException : public std::invalid_argument {
    public:
    UnknownInstructionException(const std::string &instruction)
        : std::invalid_argument("Unknown instruction: " + instruction) { }
};

class InvalidRegisterException : public std::out_of_range {
    public:
    InvalidRegisterException(register_type reg)
        : std::out_of_range("Register out of range: " + std::to_string(reg)) { }
};

class InvalidAddressException : public std::out_of_range {
    public:
    InvalidAddressException(register_type reg)
        : std::out_of_range("Address out of range: " + std::to_string(reg)) { }
};

class DivisionByZeroException : public std::domain_error {
    public:
    DivisionByZeroException()
        : std::domain_error("Division by zero") { }
};

class NoRAMException : public std::logic_error {
    public:
    NoRAMException()
        : std::logic_error("No RAM") { }
};

class NoCPUException : public std::logic_error {
    public:
    NoCPUException()
        : std::logic_error("No CPU") { }
};

class ParserException : public std::invalid_argument {
    public:
    ParserException(const std::string &cause,
                    const std::string &line,
                    std::string::size_type position)
        : std::invalid_argument("Parser error: " + cause +
                                " in: \"" + line +
                                "\":" + std::to_string(position)) { }
};

class IllegalChangeException : public std::logic_error {
    public:
    IllegalChangeException()
        : std::logic_error("OS already installed") { }
};

#endif // _COMMON_H
