#ifndef RWE_COBTHREAD_H
#define RWE_COBTHREAD_H

#include <vector>
#include <stack>

namespace rwe
{
    class CobEnvironment;

    class CobThread
    {
    public:
        enum class Status
        {
            Ready,
            Blocked,
            Finished
        };

    private:
        static const int CobTrue = 1;
        static const int CobFalse = 0;

    private:
        CobEnvironment* env;

        std::stack<int> stack;
        std::stack<std::vector<int>> locals;
        unsigned int instructionIndex{0};

        Status status{Status::Ready};

    public:
        explicit CobThread(CobEnvironment* env);

        void setEntryPoint(unsigned int functionId, const std::vector<int>& params);

        void execute();

        Status getStatus() const;

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

        // timing
        void waitForMove();

        void waitForTurn();

        void sleep();

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

        // non-commands
        int pop();
        void push(int val);

        unsigned int nextInstruction();

        void dispatchInstruction(unsigned int instruction);
    };
}

#endif
