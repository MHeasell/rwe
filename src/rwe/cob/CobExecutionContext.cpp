#include "CobExecutionContext.h"
#include <rwe/SceneManager.h>
#include <rwe/cob/CobConstants.h>
#include <rwe/cob/CobOpCode.h>
#include <rwe/cob/cob_util.h>
#include <rwe/fixed_point.h>

namespace rwe
{
    CobExecutionContext::CobExecutionContext(
        GameSimulation* sim,
        CobEnvironment* env,
        CobThread* thread,
        UnitId unitId) : sim(sim), env(env), thread(thread), unitId(unitId)
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

                    auto ticksToWait = GameTimeDelta(duration / SceneManager::TickInterval);
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

                case OpCode::GET_VALUE:
                    getValue();
                    break;
                case OpCode::GET_VALUE_WITH_ARGS:
                    getValueWithArgs();
                    break;

                case OpCode::SET_VALUE:
                    setValue();
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

    void CobExecutionContext::moveObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto position = popPosition();
        if (axis == Axis::X) // flip x-axis translations to match our right-handed coordinates
        {
            position = -position;
        }
        auto speed = popSpeed();
        sim->moveObject(unitId, getObjectName(object), axis, position, speed);
    }

    void CobExecutionContext::moveObjectNow()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto position = popPosition();
        if (axis == Axis::X) // flip x-axis translations to match our right-handed coordinates
        {
            position = -position;
        }
        sim->moveObjectNow(unitId, getObjectName(object), axis, position);
    }

    void CobExecutionContext::turnObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto angle = popAngle();
        if (axis == Axis::Z) // flip z-axis rotations to match our right-handed coordinates
        {
            angle = TaAngle(-angle.value);
        }
        auto speed = popAngularSpeed();
        sim->turnObject(unitId, getObjectName(object), axis, toRadians(angle), speed);
    }

    void CobExecutionContext::turnObjectNow()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto angle = popAngle();
        if (axis == Axis::Z) // flip z-axis rotations to match our right-handed coordinates
        {
            angle = TaAngle(-angle.value);
        }
        sim->turnObjectNow(unitId, getObjectName(object), axis, toRadians(angle));
    }

    void CobExecutionContext::spinObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto targetSpeed = popSignedAngularSpeed();
        auto acceleration = popAngularSpeed();
        sim->spinObject(unitId, getObjectName(object), axis, targetSpeed, acceleration);
    }

    void CobExecutionContext::stopSpinObject()
    {
        auto object = nextInstruction();
        auto axis = nextInstructionAsAxis();
        auto deceleration = popAngularSpeed();
        sim->stopSpinObject(unitId, getObjectName(object), axis, deceleration);
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
        sim->enableShading(unitId, getObjectName(object));
    }

    void CobExecutionContext::disableShading()
    {
        auto object = nextInstruction();
        sim->disableShading(unitId, getObjectName(object));
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

    void CobExecutionContext::getValue()
    {
        auto valueId = popValueId();
        push(getValueInternal(valueId, 0, 0, 0, 0));
    }

    void CobExecutionContext::getValueWithArgs()
    {
        auto arg4 = pop();
        auto arg3 = pop();
        auto arg2 = pop();
        auto arg1 = pop();
        auto valueId = popValueId();
        push(getValueInternal(valueId, arg1, arg2, arg3, arg4));
    }

    void CobExecutionContext::setValue()
    {
        auto newValue = pop();
        auto valueId = popValueId();
        setGetter(valueId, newValue);
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

    float CobExecutionContext::popPosition()
    {
        auto val = pop();
        return static_cast<float>(val) / 65536.0f;
    }

    float CobExecutionContext::popSpeed()
    {
        auto val = static_cast<unsigned int>(pop());
        return static_cast<float>(val) / 65536.0f;
    }

    TaAngle CobExecutionContext::popAngle()
    {
        return TaAngle(pop());
    }

    float CobExecutionContext::popAngularSpeed()
    {
        auto val = static_cast<unsigned int>(pop());
        return static_cast<float>(val) / 182.0f;
    }

    float CobExecutionContext::popSignedAngularSpeed()
    {
        auto val = pop();
        return static_cast<float>(val) / 182.0f;
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
        return env->script()->instructions.at(thread->callStack.top().instructionIndex++);
    }

    const std::string& CobExecutionContext::getObjectName(unsigned int objectId)
    {
        return env->_script->pieces.at(objectId);
    }

    int CobExecutionContext::getValueInternal(CobValueId valueId, int arg1, int arg2, int arg3, int arg4)
    {
        switch (valueId)
        {
            case CobValueId::Activation:
                return false; // TODO
            case CobValueId::StandingFireOrders:
                return 0; // TODO
            case CobValueId::StandingMoveOrders:
                return 0; // TODO
            case CobValueId::Health:
            {
                const auto& unit = sim->getUnit(unitId);
                return unit.hitPoints / unit.maxHitPoints;
            }
            case CobValueId::InBuildStance:
                return false; // TODO
            case CobValueId::Busy:
                return false; // TODO
            case CobValueId::PieceXZ:
            {
                auto pieceId = arg1;
                const auto& pieceName = getObjectName(pieceId);
                const auto& unit = sim->getUnit(unitId);
                auto pieceTransform = unit.mesh.getPieceTransform(pieceName);
                if (!pieceTransform)
                {
                    throw std::runtime_error("Unknown piece " + pieceName);
                }
                auto pos = unit.getTransform() * (*pieceTransform) * Vector3f(0.0f, 0.0f, 0.0f);
                return packCoords(pos.x, pos.z);
            }
            case CobValueId::PieceY:
            {
                auto pieceId = arg1;
                const auto& pieceName = getObjectName(pieceId);
                const auto& unit = sim->getUnit(unitId);
                auto pieceTransform = unit.mesh.getPieceTransform(pieceName);
                if (!pieceTransform)
                {
                    throw std::runtime_error("Unknown piece " + pieceName);
                }
                const auto& pos = unit.getTransform() * (*pieceTransform) * Vector3f(0.0f, 0.0f, 0.0f);
                return toFixedPoint(pos.y);
            }
            case CobValueId::UnitXZ:
            {
                auto targetUnitId = UnitId(arg1);
                const auto& pos = sim->getUnit(targetUnitId).position;
                return packCoords(pos.x, pos.z);
            }
            case CobValueId::UnitY:
            {
                auto targetUnitId = UnitId(arg1);
                const auto& pos = sim->getUnit(targetUnitId).position;
                return toFixedPoint(pos.y);
            }
            case CobValueId::UnitHeight:
            {
                auto targetUnitId = UnitId(arg1);
                return toFixedPoint(sim->getUnit(targetUnitId).height);
            }
            case CobValueId::XZAtan:
            {
                auto coords = arg1;
                auto pair = unpackCoords(coords);
                const auto& unit = sim->getUnit(unitId);
                auto result = RadiansAngle::fromUnwrappedAngle(std::atan2(pair.first, pair.second) - unit.rotation);
                return static_cast<int>(toTaAngle(result).value);
            }
            case CobValueId::XZHypot:
            {
                auto coords = arg1;
                auto pair = unpackCoords(coords);
                auto result = std::hypot(pair.first, pair.second);
                return toFixedPoint(result);
            }
            case CobValueId::Atan:
            {
                return cobAtan(arg1, arg2);
            }
            case CobValueId::Hypot:
            {
                auto a = fromFixedPoint(arg1);
                auto b = fromFixedPoint(arg2);
                auto result = std::hypot(a, b);
                return toFixedPoint(result);
            }
            case CobValueId::GroundHeight:
            {
                auto coords = arg1;
                auto pair = unpackCoords(coords);
                auto result = sim->terrain.getHeightAt(pair.first, pair.second);
                return toFixedPoint(result);
            }
            case CobValueId::BuildPercentLeft:
                return 0; // TODO
            case CobValueId::YardOpen:
                return false; // TODO
            case CobValueId::BuggerOff:
                return false; // TODO
            case CobValueId::Armored:
                return false; // TODO
            case CobValueId::VeteranLevel:
                return 0; // TODO
            case CobValueId::UnitIsOnThisComp:
                // This concept is not supported in RWE.
                // Simulation state cannot be allowed to diverge
                // between one computer and another.
                return true;
            case CobValueId::MinId:
                return 0; // TODO
            case CobValueId::MaxId:
                return sim->nextUnitId.value - 1;
            case CobValueId::MyId:
                return unitId.value;
            case CobValueId::UnitTeam:
            {
                auto targetUnitId = UnitId(arg1);
                const auto& unit = sim->getUnit(targetUnitId);
                // TODO: return player's team instead of player ID
                return unit.owner.value;
            }
            case CobValueId::UnitBuildPercentLeft:
            {
                auto targetUnitId = UnitId(arg1);
                return 0; // TODO
            }
            case CobValueId::UnitAllied:
            {
                auto targetUnitId = UnitId(arg1);

                const auto& unit = sim->getUnit(unitId);
                const auto& targetUnit = sim->getUnit(targetUnitId);
                // TODO: real allied check including teams/alliances
                return targetUnit.isOwnedBy(unit.owner);
            }
            default:
                throw std::runtime_error("Unknown unit value ID: " + std::to_string(static_cast<unsigned int>(valueId)));
        }
    }

    void CobExecutionContext::setGetter(CobValueId valueId, int value)
    {
        switch (valueId)
        {
            case CobValueId::Activation:
                return; // TODO
            case CobValueId::StandingMoveOrders:
                return; // TODO
            case CobValueId::StandingFireOrders:
                return; // TODO
            case CobValueId::InBuildStance:
                return; // TODO
            case CobValueId::Busy:
                return; // TODO
            case CobValueId::YardOpen:
                return; // TODO
            case CobValueId::BuggerOff:
                return; // TODO
            case CobValueId::Armored:
                return; // TODO
            default:
                throw std::runtime_error("Cannot set unit value with ID: " + std::to_string(static_cast<unsigned int>(valueId)));
        }
    }
}
