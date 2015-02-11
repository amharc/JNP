#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H

#include <memory>
#include <string>
#include <list>
#include "common.h"
#include "instruction.h"
#include "process.h"

namespace computer_internal {
class Assembler {
    private:
    Assembler(); // Singleton class

    class Parser {
        using pos_type = std::string::size_type;

        private:
        std::string line;
        pos_type pos;

        void require(bool condition, const std::string &why) const;
        void removeTrailingDOSEndline();
        void skipSpaces();

        public:
        Parser(const std::string &line);
        Parser(std::string &&line);
        bool hasNext() const;
        char getNext();

        template<class T>
        T parseIntegral();

        register_type parseRegister();
        memory_type parseAddress();
        number_type parseNumber();
        void end();
        std::string getWord();
    };

    static std::shared_ptr<const Instruction> compileLine(std::string &&line);
    public:
    static std::shared_ptr<Program> compile(const std::string &code);
};
} // namespace computer_internal

#endif // _ASSEMBLER_H
