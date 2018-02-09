#ifndef RWE_COBENVIRONMENT_H
#define RWE_COBENVIRONMENT_H

#include <boost/variant.hpp>
#include <memory>
#include <rwe/Cob.h>
#include <rwe/GameTime.h>
#include <rwe/UnitId.h>
#include <rwe/cob/CobThread.h>
#include <vector>


namespace rwe
{
    class GameScene;

    class CobEnvironment
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
                GameTime wakeUpTime;

                explicit Sleep(GameTime wakeUpTime) : wakeUpTime(wakeUpTime)
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

        /**
         * Emitted when a cob thread wants to send a signal to other threads.
         * The thread will go back to ready, and the signal will be broadcast.
         * If the thread was not killed by the signal it will then resume execution.
         */
        struct SignalStatus
        {
            unsigned int signal;
        };

        using Status = boost::variant<SignalStatus, BlockedStatus, FinishedStatus>;

    public:
        const CobScript* const _script;

        std::vector<int> _statics;

        std::vector<std::unique_ptr<CobThread>> threads;

        std::deque<CobThread*> readyQueue;
        std::deque<std::pair<BlockedStatus, CobThread*>> blockedQueue;

    public:
        explicit CobEnvironment(const CobScript* _script);

        CobEnvironment(const CobEnvironment& other) = delete;
        CobEnvironment& operator=(const CobEnvironment& other) = delete;
        CobEnvironment(CobEnvironment&& other) = delete;
        CobEnvironment& operator=(CobEnvironment&& other) = delete;

    public:
        int getStatic(unsigned int id);

        void setStatic(unsigned int id, int value);

        const CobScript* script();

        void createThread(unsigned int functionId, const std::vector<int>& params);

        void createThread(const std::string& functionName, const std::vector<int>& params);

        void createThread(const std::string& functionName);

        void deleteThread(const CobThread* thread);

        /**
         * Sends a signal to all threads.
         * If the signal is non-zero after being ANDed
         * with the thread's signal mask, the thread is killed.
         */
        void sendSignal(unsigned int signal);

    private:
        void removeThreadFromQueues(const CobThread* thread);
    };
}

#endif
