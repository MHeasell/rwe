#include "CobConstants.h"
#include "CobExecutionContext.h"
#include <rwe/SceneManager.h>

namespace rwe
{
    enum class OpCode
    {
        MOVE = 0x10001000,
        TURN = 0x10002000,
        SPIN = 0x10003000,
        STOP_SPIN = 0x10004000,
        SHOW = 0x10005000,
        HIDE = 0x10006000,
        CACHE = 0x10007000,
        DONT_CACHE = 0x10008000,
        MOVE_NOW = 0x1000B000,
        TURN_NOW = 0x1000C000,
        SHADE = 0x1000D000,
        DONT_SHADE = 0x1000E000,
        EMIT_SFX = 0x1000F000,

        WAIT_FOR_TURN = 0x10011000,
        WAIT_FOR_MOVE = 0x10012000,
        SLEEP = 0x10013000,

        PUSH_CONSTANT = 0x10021001,
        PUSH_LOCAL_VAR = 0x10021002,
        PUSH_STATIC = 0x10021004,
        CREATE_LOCAL_VAR = 0x10022000,
        POP_LOCAL_VAR = 0x10023002,
        POP_STATIC = 0x10023004,
        POP_STACK = 0x10024000,

        ADD = 0x10031000,
        SUB = 0x10032000,
        MUL = 0x10033000,
        DIV = 0x10034000,

        BITWISE_AND = 0x10035000,
        BITWISE_OR = 0x10036000,
        BITWISE_XOR = 0x10037000,
        BITWISE_NOT = 0x10038000,

        RAND = 0x10041000,
        GET_UNIT_VALUE = 0x10042000,
        GET = 0x10043000,

        SET_UNIT_VALUE = 0x10082000,

        SET_LESS = 0x10051000,
        SET_LESS_OR_EQUAL = 0x10052000,
        SET_GREATER = 0x10053000,
        SET_GREATER_OR_EQUAL = 0x10054000,
        SET_EQUAL = 0x10055000,
        SET_NOT_EQUAL = 0x10056000,
        LOGICAL_AND = 0x10057000,
        LOGICAL_OR = 0x10058000,
        LOGICAL_XOR = 0x10059000,
        LOGICAL_NOT = 0x1005A000,

        START_SCRIPT = 0x10061000,
        CALL_SCRIPT = 0x10062000,
        JUMP = 0x10064000,
        RETURN = 0x10065000,
        JUMP_NOT_EQUAL = 0x10066000,
        SIGNAL = 0x10067000,
        SET_SIGNAL_MASK = 0x10068000,

        EXPLODE = 0x10071000,

        SET = 0x10082000,
        ATTACH_UNIT = 0x10083000,
        DROP_UNIT = 0x10084000,
    };

    CobExecutionContext::CobExecutionContext(
        GameSimulation* sim,
        CobEnvironment* env,
        CobThread* thread,
        UnitId unitId) : sim(sim), env(env), thread(thread), unitId(unitId)
    {
    }

    CobEnvironment::Status CobExecutionContext::execute()
    {
        while (thread->instructionIndex < env->script()->instructions.size())
        {
            auto instruction = nextInstruction();
            switch (static_cast<OpCode>(instruction))
            {
                case OpCode::RAND:
                    randomNumber();
                    break;

                case OpCode::ADD:
                    add();
                    break;
                case OpCode::SUB:
                    subtract();
                    break;
                case OpCode::MUL:
                    multiply();
                    break;
                case OpCode::DIV:
                    divide();
                    break;

                case OpCode::SET_LESS:
                    compareLessThan();
                    break;
                case OpCode::SET_LESS_OR_EQUAL:
                    compareLessThanOrEqual();
                    break;
                case OpCode::SET_EQUAL:
                    compareEqual();
                    break;
                case OpCode::SET_NOT_EQUAL:
                    compareNotEqual();
                    break;
                case OpCode::SET_GREATER:
                    compareGreaterThan();
                    break;
                case OpCode::SET_GREATER_OR_EQUAL:
                    compareGreaterThanOrEqual();
                    break;

                case OpCode::JUMP:
                    jump();
                    break;
                case OpCode::JUMP_NOT_EQUAL:
                    jumpIfZero();
                    break;

                case OpCode::LOGICAL_AND:
                    logicalAnd();
                    break;
                case OpCode::LOGICAL_OR:
                    logicalOr();
                    break;
                case OpCode::LOGICAL_XOR:
                    logicalXor();
                    break;
                case OpCode::LOGICAL_NOT:
                    logicalNot();
                    break;

                case OpCode::BITWISE_AND:
                    bitwiseAnd();
                    break;
                case OpCode::BITWISE_OR:
                    bitwiseOr();
                    break;
                case OpCode::BITWISE_XOR:
                    bitwiseXor();
                    break;
                case OpCode::BITWISE_NOT:
                    bitwiseNot();
                    break;

                case OpCode::MOVE:
                    moveObject();
                    break;
                case OpCode::MOVE_NOW:
                    moveObjectNow();
                    break;
                case OpCode::TURN:
                    turnObject();
                    break;
                case OpCode::TURN_NOW:
                    turnObjectNow();
                    break;
                case OpCode::SPIN:
                    spinObject();
                    break;
                case OpCode::STOP_SPIN:
                    stopSpinObject();
                    break;
                case OpCode::EXPLODE:
                    explode();
                    break;
                case OpCode::EMIT_SFX:
                    emitSmoke();
                    break;
                case OpCode::SHOW:
                    showObject();
                    break;
                case OpCode::HIDE:
                    hideObject();
                    break;
                case OpCode::SHADE:
                    enableShading();
                    break;
                case OpCode::DONT_SHADE:
                    disableShading();
                    break;
                case OpCode::CACHE:
                    enableCaching();
                    break;
                case OpCode::DONT_CACHE:
                    disableCaching();
                    break;
                case OpCode::ATTACH_UNIT:
                    attachUnit();
                    break;
                case OpCode::DROP_UNIT:
                    detachUnit();
                    break;

                case OpCode::WAIT_FOR_MOVE:
                {
                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();

                    return CobEnvironment::BlockedStatus(CobEnvironment::BlockedStatus::Move(object, axis));
                }
                case OpCode::WAIT_FOR_TURN:
                {
                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();

                    return CobEnvironment::BlockedStatus(CobEnvironment::BlockedStatus::Turn(object, axis));
                }
                case OpCode::SLEEP:
                {
                    auto duration = pop();

                    auto ticksToWait = duration / SceneManager::TickInterval;
                    auto currentTime = sim->gameTime;

                    return CobEnvironment::BlockedStatus(CobEnvironment::BlockedStatus::Sleep(currentTime + ticksToWait));
                }

                case OpCode::CALL_SCRIPT:
                    callScript();
                    break;
                case OpCode::RETURN:
                    returnFromScript();
                    break;
                case OpCode::START_SCRIPT:
                    startScript();
                    break;

                case OpCode::SIGNAL:
                    sendSignal();
                    break;
                case OpCode::SET_SIGNAL_MASK:
                    setSignalMask();
                    break;

                case OpCode::CREATE_LOCAL_VAR:
                    createLocalVariable();
                    break;
                case OpCode::PUSH_CONSTANT:
                    pushConstant();
                    break;
                case OpCode::PUSH_LOCAL_VAR:
                    pushLocalVariable();
                    break;
                case OpCode::POP_LOCAL_VAR:
                    popLocalVariable();
                    break;
                case OpCode::PUSH_STATIC:
                    pushStaticVariable();
                    break;
                case OpCode::POP_STATIC:
                    popStaticVariable();
                    break;
                case OpCode::POP_STACK:
                    popStackOperation();
                    break;

                case OpCode::GET_UNIT_VALUE:
                    getUnitValue();
                    break;

                default:
                    throw std::runtime_error("Unsupported opcode " + std::to_string(instruction));
            }
        }

        return CobEnvironment::FinishedStatus();
    }

    void CobExecutionContext::randomNumber()
    {
        auto high = pop();
        auto low = pop();
        auto range = high - low;

        // FIXME: replace with deterministic RNG source
        auto value = (std::rand() % range) + low;
        push(value);
    }

    void CobExecutionContext::add()
    {
        auto b = pop();
        auto a = pop();
        push(a + b);
    }

    void CobExecutionContext::subtract()
    {
        auto b = pop();
        auto a = pop();
        push(a - b);
    }

    void CobExecutionContext::multiply()
    {
        auto b = pop();
        auto a = pop();
        push(a * b);
    }

    void CobExecutionContext::divide()
    {
        auto b = pop();
        auto a = pop();
        push(a / b);
    }

    void CobExecutionContext::compareLessThan()
    {
        auto b = pop();
        auto a = pop();
        push(a < b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::compareLessThanOrEqual()
    {
        auto b = pop();
        auto a = pop();
        push(a <= b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::compareEqual()
    {
        auto b = pop();
        auto a = pop();
        push(a == b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::compareNotEqual()
    {
        auto b = pop();
        auto a = pop();
        push(a != b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::compareGreaterThan()
    {
        auto b = pop();
        auto a = pop();
        push(a > b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::compareGreaterThanOrEqual()
    {
        auto b = pop();
        auto a = pop();
        push(a >= b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::jump()
    {
        auto jumpOffset = nextInstruction();
        thread->instructionIndex = jumpOffset;
    }

    void CobExecutionContext::jumpIfZero()
    {
        auto jumpOffset = nextInstruction();
        auto value = pop();
        if (value == 0)
        {
            thread->instructionIndex = jumpOffset;
        }
    }

    void CobExecutionContext::logicalAnd()
    {
        auto b = pop();
        auto a = pop();
        push(a && b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::logicalOr()
    {
        auto b = pop();
        auto a = pop();
        push(a || b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::logicalXor()
    {
        auto b = pop();
        auto a = pop();
        push(!a != !b ? CobTrue : CobFalse);
    }

    void CobExecutionContext::logicalNot()
    {
        auto v = pop();
        push(!v ? CobTrue : CobFalse);
    }

    void CobExecutionContext::bitwiseAnd()
    {
        auto b = pop();
        auto a = pop();
        push(a & b);
    }

    void CobExecutionContext::bitwiseOr()
    {
        auto b = pop();
        auto a = pop();
        push(a | b);
    }

    void CobExecutionContext::bitwiseXor()
    {
        auto b = pop();
        auto a = pop();
        push(a ^ b);
    }

    void CobExecutionContext::bitwiseNot()
    {
        auto v = pop();
        push(~v);
    }

    void CobExecutionContext::moveObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto position = popPosition();
        auto speed = popSpeed();
        sim->moveObject(unitId, getObjectName(object), axis, position, speed);
    }

    void CobExecutionContext::moveObjectNow()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto position = popPosition();
        sim->moveObjectNow(unitId, getObjectName(object), axis, position);
    }

    void CobExecutionContext::turnObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto angle = popAngle();
        auto speed = popAngularSpeed();
        sim->turnObject(unitId, getObjectName(object), axis, angle, speed);
    }

    void CobExecutionContext::turnObjectNow()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto angle = popAngle();
        sim->turnObjectNow(unitId, getObjectName(object), axis, angle);
    }

    void CobExecutionContext::spinObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstruction();
        auto targetSpeed = pop();
        auto acceleration = pop();
        // TODO: this
    }

    void CobExecutionContext::stopSpinObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstruction();
        auto deceleration = pop();
        // TODO: this
    }

    void CobExecutionContext::explode()
    {
        auto object = nextInstruction();
        auto explosionType = pop();
        // TODO: this
    }

    void CobExecutionContext::emitSmoke()
    {
        auto piece = nextInstruction();
        auto smokeType = pop();
        // TODO: this
    }

    void CobExecutionContext::showObject()
    {
        auto object = nextInstruction();
        sim->showObject(unitId, getObjectName(object));
    }

    void CobExecutionContext::hideObject()
    {
        auto object = nextInstruction();
        sim->hideObject(unitId, getObjectName(object));
    }

    void CobExecutionContext::enableShading()
    {
        auto object = nextInstruction();
        // TODO: this
    }

    void CobExecutionContext::disableShading()
    {
        auto object = nextInstruction();
        // TODO: this
    }

    void CobExecutionContext::enableCaching()
    {
        auto object = nextInstruction();
        // TODO: this
    }

    void CobExecutionContext::disableCaching()
    {
        auto object = nextInstruction();
        // TODO: this
    }

    void CobExecutionContext::attachUnit()
    {
        auto piece = pop();
        auto unit = pop();
        // TODO: this
    }

    void CobExecutionContext::detachUnit()
    {
        auto unit = pop();
        // TODO: this
    }

    void CobExecutionContext::returnFromScript()
    {
        auto returnValue = pop();
        thread->instructionIndex = pop();
        thread->locals.pop();
        // TODO: do something with the return value
    }

    void CobExecutionContext::callScript()
    {
        auto functionId = nextInstruction();
        auto paramCount = nextInstruction();

        // set up parameters as local variables
        thread->locals.emplace(paramCount);
        for (unsigned int i = 0; i < paramCount; ++i)
        {
            auto param = pop();
            thread->locals.top()[i] = param;
        }

        push(thread->instructionIndex);

        const auto& functionInfo = env->script()->functions.at(functionId);
        thread->instructionIndex = functionInfo.address;
    }

    void CobExecutionContext::startScript()
    {
        auto functionId = nextInstruction();
        auto paramCount = nextInstruction();

        std::vector<int> params(paramCount);
        for (unsigned int i = 0; i < paramCount; ++i)
        {
            params[i] = pop();
        }

        env->createThread(functionId, params);
    }

    void CobExecutionContext::sendSignal()
    {
        auto signal = pop();
        // TODO: this
    }

    void CobExecutionContext::setSignalMask()
    {
        auto mask = pop();
        // TODO: this
    }

    void CobExecutionContext::createLocalVariable()
    {
        thread->locals.top().emplace_back();
    }

    void CobExecutionContext::pushConstant()
    {
        auto constant = nextInstruction();
        push(constant);
    }

    void CobExecutionContext::pushLocalVariable()
    {
        auto variableId = nextInstruction();
        push(thread->locals.top().at(variableId));
    }

    void CobExecutionContext::popLocalVariable()
    {
        auto variableId = nextInstruction();
        auto value = pop();
        thread->locals.top().at(variableId) = value;
    }

    void CobExecutionContext::pushStaticVariable()
    {
        auto variableId = nextInstruction();
        push(env->getStatic(variableId));
    }

    void CobExecutionContext::popStaticVariable()
    {
        auto variableId = nextInstruction();
        auto value = pop();
        env->setStatic(variableId, value);
    }

    void CobExecutionContext::popStackOperation()
    {
        pop();
    }

    void CobExecutionContext::getUnitValue()
    {
        auto valueId = pop();
        // TODO: retrieve actual value
        push(0);
    }

    void CobExecutionContext::setUnitValue()
    {
        auto newValue = pop();
        auto valueId = pop();
        // TODO: actually set the value
    }

    int CobExecutionContext::pop()
    {
        auto v = thread->stack.top();
        thread->stack.pop();
        return v;
    }

    float CobExecutionContext::popPosition()
    {
        auto val = pop();
        return static_cast<float>(val) / 163840.0f;
    }

    float CobExecutionContext::popSpeed()
    {
        auto val = static_cast<unsigned int>(pop());
        return static_cast<float>(val) / 163840.0f;
    }

    float CobExecutionContext::popAngle()
    {
        auto val = static_cast<unsigned int>(pop());
        return static_cast<float>(val) / 182.0f;
    }

    float CobExecutionContext::popAngularSpeed()
    {
        auto val = static_cast<unsigned int>(pop());
        return static_cast<float>(val) / 182.0f;
    }

    void CobExecutionContext::push(int val)
    {
        thread->stack.push(val);
    }

    Axis CobExecutionContext::nextInstructionAsAxis()
    {
        auto val = nextInstruction();
        switch (val)
        {
            case 0:
                return Axis::X;
            case 1:
                return Axis::Y;
            case 2:
                return Axis::Z;
            default:
                throw std::runtime_error("Invalid axis: " + std::to_string(val));
        }
    }

    unsigned int CobExecutionContext::nextInstruction()
    {
        return env->script()->instructions.at(thread->instructionIndex++);
    }

    const std::string& CobExecutionContext::getObjectName(unsigned int objectId)
    {
        return env->_script->pieces.at(objectId);
    }
}
