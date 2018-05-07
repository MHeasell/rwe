#include <fstream>
#include <iomanip>
#include <iostream>
#include <rwe/Cob.h>
#include <rwe/_3do.h>
#include <rwe/cob/CobOpCode.h>
#include <rwe/optional_io.h>
#include <vector>

namespace rwe
{
    std::optional<const char*> getInstructionName(uint32_t instruction)
    {
        switch (static_cast<OpCode>(instruction))
        {
            case OpCode::MOVE:
                return "MOVE";
            case OpCode::TURN:
                return "TURN";
            case OpCode::SPIN:
                return "SPIN";
            case OpCode::STOP_SPIN:
                return "STOP_SPIN";
            case OpCode::SHOW:
                return "SHOW";
            case OpCode::HIDE:
                return "HIDE";
            case OpCode::CACHE:
                return "CACHE";
            case OpCode::DONT_CACHE:
                return "DONT_CACHE";
            case OpCode::MOVE_NOW:
                return "MOVE_NOW";
            case OpCode::TURN_NOW:
                return "TURN_NOW";
            case OpCode::SHADE:
                return "SHADE";
            case OpCode::DONT_SHADE:
                return "DONT_SHADE";
            case OpCode::EMIT_SFX:
                return "EMIT_SFX";

            case OpCode::WAIT_FOR_TURN:
                return "WAIT_FOR_TURN";
            case OpCode::WAIT_FOR_MOVE:
                return "WAIT_FOR_MOVE";
            case OpCode::SLEEP:
                return "SLEEP";

            case OpCode::PUSH_CONSTANT:
                return "PUSH_CONSTANT";
            case OpCode::PUSH_LOCAL_VAR:
                return "PUSH_LOCAL_VAR";
            case OpCode::PUSH_STATIC:
                return "PUSH_STATIC";
            case OpCode::CREATE_LOCAL_VAR:
                return "CREATE_LOCAL_VAR";
            case OpCode::POP_LOCAL_VAR:
                return "POP_LOCAL_VAR";
            case OpCode::POP_STATIC:
                return "POP_STATIC";
            case OpCode::POP_STACK:
                return "POP_STACK";

            case OpCode::ADD:
                return "ADD";
            case OpCode::SUB:
                return "SUB";
            case OpCode::MUL:
                return "MUL";
            case OpCode::DIV:
                return "DIV";

            case OpCode::BITWISE_AND:
                return "BITWISE_AND";
            case OpCode::BITWISE_OR:
                return "BITWISE_OR";
            case OpCode::BITWISE_XOR:
                return "BITWISE_XOR";
            case OpCode::BITWISE_NOT:
                return "BITWISE_NOT";

            case OpCode::RAND:
                return "RAND";
            case OpCode::GET_VALUE:
                return "GET_VALUE";
            case OpCode::GET_VALUE_WITH_ARGS:
                return "GET_VALUE_WITH_ARGS";

            case OpCode::SET_VALUE:
                return "SET_VALUE";

            case OpCode::SET_LESS:
                return "SET_LESS";
            case OpCode::SET_LESS_OR_EQUAL:
                return "SET_LESS_OR_EQUAL";
            case OpCode::SET_GREATER:
                return "SET_GREATER";
            case OpCode::SET_GREATER_OR_EQUAL:
                return "SET_GREATER_OR_EQUAL";
            case OpCode::SET_EQUAL:
                return "SET_EQUAL";
            case OpCode::SET_NOT_EQUAL:
                return "SET_NOT_EQUAL";
            case OpCode::LOGICAL_AND:
                return "LOGICAL_AND";
            case OpCode::LOGICAL_OR:
                return "LOGICAL_OR";
            case OpCode::LOGICAL_XOR:
                return "LOGICAL_XOR";
            case OpCode::LOGICAL_NOT:
                return "LOGICAL_NOT";

            case OpCode::START_SCRIPT:
                return "START_SCRIPT";
            case OpCode::CALL_SCRIPT:
                return "CALL_SCRIPT";
            case OpCode::JUMP:
                return "JUMP";
            case OpCode::RETURN:
                return "RETURN";
            case OpCode::JUMP_IF_ZERO:
                return "JUMP_IF_ZERO";
            case OpCode::SIGNAL:
                return "SIGNAL";
            case OpCode::SET_SIGNAL_MASK:
                return "SET_SIGNAL_MASK";

            case OpCode::EXPLODE:
                return "EXPLODE";

            case OpCode::ATTACH_UNIT:
                return "ATTACH_UNIT";
            case OpCode::DROP_UNIT:
                return "DROP_UNIT";

            default:
                return std::nullopt;
        }
    }
}


template <typename T>
class CobInstructionPrinter
{
private:
    T begin;
    T it;
    const T end;

public:
    CobInstructionPrinter(T begin, const T end) : begin(begin), it(begin), end(end)
    {
    }

    void printInstructions()
    {
        for (auto instruction = next(); instruction; instruction = next())
        {
            std::cout << "  " << std::setw(4) << instruction->first << ": ";

            auto name = rwe::getInstructionName(instruction->second);
            if (name)
            {
                std::cout << *name;
            }
            else
            {
                std::cout << instruction->second;
            }

            std::cout << std::endl;
        }
    }

private:
    std::optional<std::pair<unsigned int, uint32_t>> next()
    {
        if (it == end)
        {
            return std::nullopt;
        }

        auto count = it - begin;

        return std::pair<unsigned int, uint32_t>(count, *it++);
    }
};

void printCob(const rwe::CobScript& cob)
{
    std::cout << "Pieces:" << std::endl;
    for (unsigned int i = 0; i < cob.pieces.size(); ++i)
    {
        std::cout << "  " << std::setw(2) << i << ": " << cob.pieces[i] << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Static Variables: " << cob.staticVariableCount << std::endl;

    std::cout << std::endl;

    std::cout << "Functions:" << std::endl;
    for (unsigned int i = 0; i < cob.functions.size(); ++i)
    {
        const auto& f = cob.functions[i];
        std::cout << "  " << std::setw(2) << i << ": " << f.name << " (" << f.address << ")" << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Instructions: " << std::endl;

    CobInstructionPrinter<std::vector<unsigned int>::const_iterator>(cob.instructions.begin(), cob.instructions.end()).printInstructions();
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Specify a cob file to dump." << std::endl;
        return 1;
    }

    std::string filename(argv[1]);

    std::ifstream fh(filename, std::ios::binary);

    auto script = rwe::parseCob(fh);

    printCob(script);

    return 0;
}
