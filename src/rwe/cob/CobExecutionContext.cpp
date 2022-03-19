#include "CobExecutionContext.h"
#include <random>
#include <rwe/cob/CobConstants.h>
#include <rwe/cob/CobOpCode.h>
#include <rwe/cob/cob_util.h>
#include <stdexcept>
#include <variant>

namespace rwe
{
    std::variant<int, CobEnvironment::QueryStatus::Query> getValueInternal(CobValueId valueId, int arg1, int arg2, int /*arg3*/, int /*arg4*/)
    {
        switch (valueId)
        {
            case CobValueId::Activation:
                return CobEnvironment::QueryStatus::Activation{};
            case CobValueId::StandingFireOrders:
                return CobEnvironment::QueryStatus::StandingFireOrders{};
            case CobValueId::StandingMoveOrders:
                return CobEnvironment::QueryStatus::StandingMoveOrders{};
            case CobValueId::Health:
                return CobEnvironment::QueryStatus::Health{};
            case CobValueId::InBuildStance:
                return CobEnvironment::QueryStatus::InBuildStance{};
            case CobValueId::Busy:
                return CobEnvironment::QueryStatus::Busy{};
            case CobValueId::PieceXZ:
                return CobEnvironment::QueryStatus::PieceXZ{arg1};
            case CobValueId::PieceY:
                return CobEnvironment::QueryStatus::PieceY{arg1};
            case CobValueId::UnitXZ:
                return CobEnvironment::QueryStatus::UnitXZ{UnitId(arg1)};
            case CobValueId::UnitY:
                return CobEnvironment::QueryStatus::UnitY{UnitId(arg1)};
            case CobValueId::UnitHeight:
                return CobEnvironment::QueryStatus::UnitHeight{UnitId(arg1)};
            case CobValueId::XZAtan:
                return CobEnvironment::QueryStatus::XZAtan{arg1};
            case CobValueId::XZHypot:
            {
                auto coords = arg1;
                auto pair = cobUnpackCoords(coords);
                auto result = cobHypot(pair.first, pair.second);
                return result.value;
            }
            case CobValueId::Atan:
            {
                return cobAtan(arg1, arg2);
            }
            case CobValueId::Hypot:
            {
                auto a = CobPosition(arg1);
                auto b = CobPosition(arg2);
                auto result = cobHypot(a, b);
                return result.value;
            }
            case CobValueId::GroundHeight:
                return CobEnvironment::QueryStatus::GroundHeight{arg1};
            case CobValueId::BuildPercentLeft:
                return CobEnvironment::QueryStatus::BuildPercentLeft{};
            case CobValueId::YardOpen:
                return CobEnvironment::QueryStatus::YardOpen{};
            case CobValueId::BuggerOff:
                return CobEnvironment::QueryStatus::BuggerOff{};
            case CobValueId::Armored:
                return CobEnvironment::QueryStatus::Armored{};
            case CobValueId::VeteranLevel:
                return CobEnvironment::QueryStatus::VeteranLevel{};
            case CobValueId::UnitIsOnThisComp:
                // This concept is not supported in RWE.
                // Simulation state cannot be allowed to diverge
                // between one computer and another.
                return true;
            case CobValueId::MinId:
                return CobEnvironment::QueryStatus::MinId{};
            case CobValueId::MaxId:
                return CobEnvironment::QueryStatus::MaxId{};
            case CobValueId::MyId:
                return CobEnvironment::QueryStatus::MyId{};
            case CobValueId::UnitTeam:
                return CobEnvironment::QueryStatus::UnitTeam{UnitId(arg1)};
            case CobValueId::UnitBuildPercentLeft:
                return CobEnvironment::QueryStatus::UnitBuildPercentLeft{UnitId(arg1)};
            case CobValueId::UnitAllied:
                return CobEnvironment::QueryStatus::UnitAllied{UnitId(arg1)};
            default:
                throw std::runtime_error("Unknown unit value ID: " + std::to_string(static_cast<unsigned int>(valueId)));
        }
    }

    CobEnvironment::SetQueryStatus::Query setGetter(CobValueId valueId, int value)
    {
        switch (valueId)
        {
            case CobValueId::Activation:
                return CobEnvironment::SetQueryStatus::Activation{value != 0};
            case CobValueId::StandingMoveOrders:
                return CobEnvironment::SetQueryStatus::StandingMoveOrders{value};
            case CobValueId::StandingFireOrders:
                return CobEnvironment::SetQueryStatus::StandingFireOrders{value};
            case CobValueId::InBuildStance:
                return CobEnvironment::SetQueryStatus::InBuildStance{value != 0};
            case CobValueId::Busy:
                return CobEnvironment::SetQueryStatus::Busy{value != 0};
            case CobValueId::YardOpen:
                return CobEnvironment::SetQueryStatus::YardOpen{value != 0};
            case CobValueId::BuggerOff:
                return CobEnvironment::SetQueryStatus::BuggerOff{value != 0};
            case CobValueId::Armored:
                return CobEnvironment::SetQueryStatus::Armored{value != 0};
            default:
                throw std::runtime_error("Cannot set unit value with ID: " + std::to_string(static_cast<unsigned int>(valueId)));
        }
    }

    CobExecutionContext::CobExecutionContext(
        CobEnvironment* env,
        CobThread* thread,
        UnitId unitId) : env(env), thread(thread), unitId(unitId)
    {
    }

    CobEnvironment::Status CobExecutionContext::execute()
    {
        while (!thread->callStack.empty())
        {
            auto instruction = nextInstruction();
            switch (static_cast<OpCode>(instruction))
            {
                case OpCode::RAND:
                {
                    auto high = pop();
                    auto low = pop();
                    return CobEnvironment::QueryStatus{CobEnvironment::QueryStatus::Random{low, high}};
                }
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
                case OpCode::JUMP_IF_ZERO:
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
                {
                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();
                    auto position = popPosition();
                    auto speed = popSpeed();
                    return CobEnvironment::PieceCommandStatus{
                        object,
                        CobEnvironment::PieceCommandStatus::Move{axis, position, speed}};
                }
                case OpCode::MOVE_NOW:
                {
                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();
                    auto position = popPosition();
                    return CobEnvironment::PieceCommandStatus{
                        object,
                        CobEnvironment::PieceCommandStatus::Move{axis, position}};
                }
                case OpCode::TURN:
                {
                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();
                    auto angle = popAngle();
                    auto speed = popAngularSpeed();
                    return CobEnvironment::PieceCommandStatus{
                        object,
                        CobEnvironment::PieceCommandStatus::Turn{axis, angle, speed}};
                }
                case OpCode::TURN_NOW:
                {
                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();
                    auto angle = popAngle();
                    return CobEnvironment::PieceCommandStatus{
                        object,
                        CobEnvironment::PieceCommandStatus::Turn{axis, angle}};
                }
                case OpCode::SPIN:
                {
                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();
                    auto targetSpeed = popAngularSpeed();
                    auto acceleration = popAngularSpeed();
                    return CobEnvironment::PieceCommandStatus{
                        object,
                        CobEnvironment::PieceCommandStatus::Spin{axis, targetSpeed, acceleration}};
                }
                case OpCode::STOP_SPIN:
                {

                    auto object = nextInstruction();
                    auto axis = nextInstructionAsAxis();
                    auto deceleration = popAngularSpeed();
                    return CobEnvironment::PieceCommandStatus{
                        object,
                        CobEnvironment::PieceCommandStatus::StopSpin{axis, deceleration}};
                }
                case OpCode::EXPLODE:
                    explode();
                    break;
                case OpCode::EMIT_SFX:
                {
                    auto object = nextInstruction();
                    auto sfxType = popSfxType();
                    return CobEnvironment::PieceCommandStatus{
                        object,
                        CobEnvironment::PieceCommandStatus::EmitSfx{sfxType}};
                }
                case OpCode::SHOW:
                {
                    auto object = nextInstruction();
                    return CobEnvironment::PieceCommandStatus{object, CobEnvironment::PieceCommandStatus::Show()};
                }
                case OpCode::HIDE:
                {
                    auto object = nextInstruction();
                    return CobEnvironment::PieceCommandStatus{object, CobEnvironment::PieceCommandStatus::Hide()};
                }
                case OpCode::SHADE:
                {
                    auto object = nextInstruction();
                    return CobEnvironment::PieceCommandStatus{object, CobEnvironment::PieceCommandStatus::EnableShading()};
                }
                case OpCode::DONT_SHADE:
                {
                    auto object = nextInstruction();
                    return CobEnvironment::PieceCommandStatus{object, CobEnvironment::PieceCommandStatus::DisableShading()};
                }
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
                    auto duration = popSleepDuration();
                    return CobEnvironment::SleepStatus{duration};
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

                case OpCode::GET_VALUE:
                {
                    auto valueId = popValueId();
                    auto value = getValueInternal(valueId, 0, 0, 0, 0);
                    if (auto v = std::get_if<CobEnvironment::QueryStatus::Query>(&value); v != nullptr)
                    {
                        return CobEnvironment::QueryStatus{*v};
                    }
                    push(std::get<int>(value));
                    break;
                }
                case OpCode::GET_VALUE_WITH_ARGS:
                {
                    auto arg4 = pop();
                    auto arg3 = pop();
                    auto arg2 = pop();
                    auto arg1 = pop();
                    auto valueId = popValueId();
                    auto value = getValueInternal(valueId, arg1, arg2, arg3, arg4);
                    if (auto v = std::get_if<CobEnvironment::QueryStatus::Query>(&value); v != nullptr)
                    {
                        return CobEnvironment::QueryStatus{*v};
                    }
                    push(std::get<int>(value));
                    break;
                }
                case OpCode::SET_VALUE:
                {
                    auto newValue = pop();
                    auto valueId = popValueId();
                    return CobEnvironment::SetQueryStatus{setGetter(valueId, newValue)};
                }
                default:
                    throw std::runtime_error("Unsupported opcode " + std::to_string(instruction));
            }
        }

        return CobEnvironment::FinishedStatus();
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
        thread->callStack.top().instructionIndex = jumpOffset;
    }

    void CobExecutionContext::jumpIfZero()
    {
        auto jumpOffset = nextInstruction();
        auto value = pop();
        if (value == 0)
        {
            thread->callStack.top().instructionIndex = jumpOffset;
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

    void CobExecutionContext::explode()
    {
        /*auto object = */ nextInstruction();
        /*auto explosionType = */ pop();
        // TODO: this
    }

    void CobExecutionContext::enableCaching()
    {
        nextInstruction(); // object
        // do nothing, RWE does not have the concept of caching
    }

    void CobExecutionContext::disableCaching()
    {
        nextInstruction(); // object
        // do nothing, RWE does not have the concept of caching
    }

    void CobExecutionContext::attachUnit()
    {
        /*auto piece = */ pop();
        /*auto unit = */ pop();
        // TODO: this
    }

    void CobExecutionContext::detachUnit()
    {
        /*auto unit = */ pop();
        // TODO: this
    }

    void CobExecutionContext::returnFromScript()
    {
        thread->returnValue = pop();
        thread->returnLocals = thread->callStack.top().locals;
        thread->callStack.pop();
    }

    void CobExecutionContext::callScript()
    {
        auto functionId = nextInstruction();
        auto paramCount = nextInstruction();

        // collect up the parameters
        std::vector<int> params(paramCount);
        for (unsigned int i = 0; i < paramCount; ++i)
        {
            params[i] = pop();
        }

        const auto& functionInfo = env->script()->functions.at(functionId);
        thread->callStack.emplace(functionInfo.address, params);
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

        env->createThread(functionId, params, thread->signalMask);
    }

    void CobExecutionContext::sendSignal()
    {
        auto signal = popSignal();
        env->sendSignal(signal);
    }

    void CobExecutionContext::setSignalMask()
    {
        auto mask = popSignalMask();
        thread->signalMask = mask;
    }

    void CobExecutionContext::createLocalVariable()
    {
        if (thread->callStack.top().localCount == thread->callStack.top().locals.size())
        {
            thread->callStack.top().locals.emplace_back();
        }
        thread->callStack.top().localCount += 1;
    }

    void CobExecutionContext::pushConstant()
    {
        auto constant = nextInstruction();
        push(constant);
    }

    void CobExecutionContext::pushLocalVariable()
    {
        auto variableId = nextInstruction();
        push(thread->callStack.top().locals.at(variableId));
    }

    void CobExecutionContext::popLocalVariable()
    {
        auto variableId = nextInstruction();
        auto value = pop();
        thread->callStack.top().locals.at(variableId) = value;
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

    CobEnvironment::SetQueryStatus CobExecutionContext::setValue()
    {
        auto newValue = pop();
        auto valueId = popValueId();
        return CobEnvironment::SetQueryStatus{setGetter(valueId, newValue)};
    }

    int CobExecutionContext::pop()
    {
        // Malformed scripts may attempt to pop when the stack is empty.
        // For example see Github issue #56.
        if (thread->stack.empty())
        {
            return 0;
        }

        auto v = thread->stack.top();
        thread->stack.pop();
        return v;
    }

    CobSleepDuration CobExecutionContext::popSleepDuration()
    {
        return CobSleepDuration(pop());
    }

    CobPosition CobExecutionContext::popPosition()
    {
        return CobPosition(pop());
    }

    CobSpeed CobExecutionContext::popSpeed()
    {
        return CobSpeed(pop());
    }

    CobAngle CobExecutionContext::popAngle()
    {
        return CobAngle(pop());
    }

    CobAngularSpeed CobExecutionContext::popAngularSpeed()
    {
        return CobAngularSpeed(pop());
    }

    unsigned int CobExecutionContext::popSignal()
    {
        return static_cast<unsigned int>(pop());
    }

    unsigned int CobExecutionContext::popSignalMask()
    {
        return static_cast<unsigned int>(pop());
    }

    CobValueId CobExecutionContext::popValueId()
    {
        return static_cast<CobValueId>(pop());
    }

    CobSfxType CobExecutionContext::popSfxType()
    {
        return static_cast<CobSfxType>(pop());
    }

    void CobExecutionContext::push(int val)
    {
        thread->stack.push(val);
    }

    CobAxis CobExecutionContext::nextInstructionAsAxis()
    {
        auto val = nextInstruction();
        switch (val)
        {
            case 0:
                return CobAxis::X;
            case 1:
                return CobAxis::Y;
            case 2:
                return CobAxis::Z;
            default:
                throw std::runtime_error("Invalid axis: " + std::to_string(val));
        }
    }

    unsigned int CobExecutionContext::nextInstruction()
    {
        return env->script()->instructions.at(thread->callStack.top().instructionIndex++);
    }
}
