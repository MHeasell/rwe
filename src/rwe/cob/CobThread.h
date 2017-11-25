#ifndef RWE_COBTHREAD_H
#define RWE_COBTHREAD_H

#include <boost/variant.hpp>
#include <rwe/util.h>
#include <stack>
#include <vector>

namespace rwe
{
    class CobEnvironment;

    class CobThread
    {
    public:
        struct BlockedStatus
        {
            struct Move
            {
                unsigned int object;
                Axis axis;

                Move(unsigned int object, Axis axis) : object(object), axis(axis)
                {
                }
            };

            struct Turn
            {
                unsigned int object;
                Axis axis;

                Turn(unsigned int object, Axis axis) : object(object), axis(axis)
                {
                }
            };

            struct Sleep
            {
                unsigned int wakeUpTime;

                explicit Sleep(unsigned int wakeUpTime) : wakeUpTime(wakeUpTime)
                {
                }
            };

            using Condition = boost::variant<Move, Turn, Sleep>;

            Condition condition;

        public:
            explicit BlockedStatus(const Condition& condition) : condition(condition) {}
        };
        struct FinishedStatus
        {
        };

        using Status = boost::variant<BlockedStatus, FinishedStatus>;

    private:
        static const int CobTrue = 1;
        static const int CobFalse = 0;

    private:
        std::string name;
        CobEnvironment* env;

        std::stack<int> stack;
        std::stack<std::vector<int>> locals;
        unsigned int instructionIndex{0};

    public:
        explicit CobThread(CobEnvironment* env);
        CobThread(const std::string& name, CobEnvironment* env);

        void setEntryPoint(unsigned int functionId, const std::vector<int>& params);

        Status execute();

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

        void setUnitValue();

        // non-commands
        int pop();
        float popPosition();
        float popSpeed();
        float popAngle();
        float popAngularSpeed();
        void push(int val);

        unsigned int nextInstruction();
        Axis nextInstructionAsAxis();
    };
}

#endif
