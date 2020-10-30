#pragma once

#include <rwe/cob/CobAngularSpeed.h>
#include <rwe/cob/CobEnvironment.h>
#include <rwe/cob/CobPosition.h>
#include <rwe/cob/CobSleepDuration.h>
#include <rwe/cob/CobSpeed.h>
#include <rwe/cob/CobValueId.h>
#include <rwe/sim/GameSimulation.h>

namespace rwe
{
    class GameScene;

    class CobExecutionContext
    {
    private:
        GameScene* const scene;
        GameSimulation* const sim;
        CobEnvironment* const env;
        CobThread* const thread;
        const UnitId unitId;

    public:
        CobExecutionContext(GameScene* scene, GameSimulation* sim, CobEnvironment* env, CobThread* thread, UnitId unitId);

        CobEnvironment::Status execute();

    private:
        // utility
        void randomNumber();

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

        void emitSmoke();

        void showObject();

        void hideObject();

        void enableShading();

        void disableShading();

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

        void setValue();

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
        void push(int val);

        unsigned int nextInstruction();
        CobAxis nextInstructionAsAxis();

        const std::string& getObjectName(unsigned int objectId);

        void setGetter(CobValueId valueId, int value);
    };
}
