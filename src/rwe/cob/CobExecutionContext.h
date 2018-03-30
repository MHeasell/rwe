#ifndef RWE_COBEXECUTIONCONTEXT_H
#define RWE_COBEXECUTIONCONTEXT_H

#include <rwe/GameSimulation.h>
#include <rwe/cob/CobEnvironment.h>
#include <rwe/cob/CobValueId.h>

namespace rwe
{
    class CobExecutionContext
    {
    private:
        GameSimulation* const sim;
        CobEnvironment* const env;
        CobThread* const thread;
        const UnitId unitId;

    public:
        CobExecutionContext(GameSimulation* sim, CobEnvironment* env, CobThread* thread, UnitId unitId);

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
        void moveObject();

        void moveObjectNow();

        void turnObject();

        void turnObjectNow();

        void spinObject();

        void stopSpinObject();

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

        void getUnitValue();

        void getWithArgs();

        void setUnitValue();

        // non-commands
        int pop();

        float popPosition();
        float popSpeed();
        TaAngle popAngle();
        float popAngularSpeed();
        float popSignedAngularSpeed();
        unsigned int popSignal();
        unsigned int popSignalMask();
        CobValueId popValueId();
        void push(int val);

        unsigned int nextInstruction();
        Axis nextInstructionAsAxis();

        const std::string& getObjectName(unsigned int objectId);

        int getGetter(CobValueId valueId, int arg1, int arg2, int arg3, int arg4);
        void setGetter(CobValueId valueId, int value);
    };
}

#endif
