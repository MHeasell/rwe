#pragma once

#include <rwe/cob/CobAngularSpeed.h>
#include <rwe/cob/CobEnvironment.h>
#include <rwe/cob/CobPosition.h>
#include <rwe/cob/CobSfxType.h>
#include <rwe/cob/CobSleepDuration.h>
#include <rwe/cob/CobSpeed.h>
#include <rwe/cob/CobValueId.h>

namespace rwe
{
    class CobExecutionContext
    {
    private:
        CobEnvironment* const env;
        CobThread* const thread;
        const UnitId unitId;

    public:
        CobExecutionContext(CobEnvironment* env, CobThread* thread, UnitId unitId);

        CobEnvironment::Status execute();

    private:
        // arithmetic
        void add();

        void subtract();

        void multiply();

        void divide();

        // comparision
        void compareLessThan();

        void compareLessThanOrEqual();

        void compareEqual();

        void compareNotEqual();

        void compareGreaterThan();

        void compareGreaterThanOrEqual();

        // control flow
        void jump();

        void jumpIfZero();

        // boolean logic
        void logicalAnd();

        void logicalOr();

        void logicalXor();

        void logicalNot();

        // bitwise logic
        void bitwiseAnd();

        void bitwiseOr();

        void bitwiseXor();

        void bitwiseNot();

        // control object pieces
        void explode();

        void enableCaching();

        void disableCaching();

        void attachUnit();

        void detachUnit();

        // script dispatch and return
        void returnFromScript();

        void callScript();

        void startScript();

        // signalling
        void sendSignal();

        void setSignalMask();

        // variables
        void createLocalVariable();

        void pushConstant();

        void pushLocalVariable();

        void popLocalVariable();

        void pushStaticVariable();

        void popStaticVariable();

        void popStackOperation();

        CobEnvironment::SetQueryStatus setValue();

        // non-commands
        int pop();

        CobSleepDuration popSleepDuration();
        CobPosition popPosition();
        CobSpeed popSpeed();
        CobAngle popAngle();
        CobAngularSpeed popAngularSpeed();
        unsigned int popSignal();
        unsigned int popSignalMask();
        CobValueId popValueId();
        CobSfxType popSfxType();
        void push(int val);

        unsigned int nextInstruction();
        CobAxis nextInstructionAsAxis();
    };
}
