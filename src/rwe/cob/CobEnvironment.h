#pragma once

#include <memory>
#include <optional>
#include <rwe/cob/CobAngle.h>
#include <rwe/cob/CobAngularSpeed.h>
#include <rwe/cob/CobAxis.h>
#include <rwe/cob/CobPosition.h>
#include <rwe/cob/CobSleepDuration.h>
#include <rwe/cob/CobSpeed.h>
#include <rwe/cob/CobThread.h>
#include <rwe/cob/CobTime.h>
#include <rwe/io/cob/Cob.h>
#include <rwe/sim/UnitId.h>
#include <variant>
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
                CobAxis axis;

                Move(unsigned int object, CobAxis axis) : object(object), axis(axis)
                {
                }
            };

            struct Turn
            {
                unsigned int object;
                CobAxis axis;

                Turn(unsigned int object, CobAxis axis) : object(object), axis(axis)
                {
                }
            };

            using Condition = std::variant<Move, Turn>;

            Condition condition;

        public:
            explicit BlockedStatus(const Condition& condition) : condition(condition) {}
        };

        struct SleepStatus
        {
            CobSleepDuration duration;
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

        struct PieceCommandStatus
        {
            struct Move
            {
                CobAxis axis;
                CobPosition position;
                std::optional<CobSpeed> speed;
            };
            struct Turn
            {
                CobAxis axis;
                CobAngle angle;
                std::optional<CobAngularSpeed> speed;
            };
            struct Spin
            {
                CobAxis axis;
                CobAngularSpeed targetSpeed;
                CobAngularSpeed acceleration;
            };
            struct StopSpin
            {
                CobAxis axis;
                CobAngularSpeed deceleration;
            };
            struct Show
            {
            };
            struct Hide
            {
            };
            struct EnableShading
            {
            };
            struct DisableShading
            {
            };
            using CommandType = std::variant<Move, Turn, Spin, StopSpin, Show, Hide, EnableShading, DisableShading>;

            unsigned int piece;
            CommandType command;
        };

        struct QueryStatus
        {
            struct Random
            {
                int low;
                int high;
            };
            struct Activation
            {
            };
            struct StandingFireOrders
            {
            };
            struct StandingMoveOrders
            {
            };
            struct Health
            {
            };
            struct InBuildStance
            {
            };
            struct Busy
            {
            };
            struct PieceXZ
            {
                int piece;
            };
            struct PieceY
            {
                int piece;
            };
            struct UnitXZ
            {
                UnitId targetUnitId;
            };
            struct UnitY
            {
                UnitId targetUnitId;
            };
            struct UnitHeight
            {
                UnitId targetUnitId;
            };
            struct XZAtan
            {
                int32_t coords;
            };
            struct GroundHeight
            {
                int32_t coords;
            };
            struct BuildPercentLeft
            {
            };
            struct YardOpen
            {
            };
            struct BuggerOff
            {
            };
            struct Armored
            {
            };
            struct VeteranLevel
            {
            };
            struct MinId
            {
            };
            struct MaxId
            {
            };
            struct MyId
            {
            };
            struct UnitTeam
            {
                UnitId targetUnitId;
            };
            struct UnitBuildPercentLeft
            {
                UnitId targetUnitId;
            };
            struct UnitAllied
            {
                UnitId targetUnitId;
            };

            using Query = std::variant<Random, Activation, StandingFireOrders, StandingMoveOrders, Health, InBuildStance, Busy, PieceXZ, PieceY, UnitXZ, UnitY, UnitHeight, XZAtan, GroundHeight, BuildPercentLeft, YardOpen, BuggerOff, Armored, VeteranLevel, MinId, MaxId, MyId, UnitTeam, UnitBuildPercentLeft, UnitAllied>;

            Query query;
        };

        struct SetQueryStatus
        {
            struct Activation
            {
                bool value;
            };
            struct StandingMoveOrders
            {
                int value;
            };
            struct StandingFireOrders
            {
                int value;
            };
            struct InBuildStance
            {
                bool value;
            };
            struct Busy
            {
                bool value;
            };
            struct YardOpen
            {
                bool value;
            };
            struct BuggerOff
            {
                bool value;
            };
            struct Armored
            {
                bool value;
            };

            using Query = std::variant<Activation, StandingMoveOrders, StandingFireOrders, InBuildStance, Busy, YardOpen, BuggerOff, Armored>;

            Query query;
        };

        using Status = std::variant<SignalStatus, PieceCommandStatus, BlockedStatus, SleepStatus, QueryStatus, SetQueryStatus, FinishedStatus>;

    public:
        const CobScript* const _script;

        std::vector<int> _statics;

        std::vector<std::unique_ptr<CobThread>> threads;

        std::deque<CobThread*> readyQueue;
        std::deque<std::pair<BlockedStatus, CobThread*>> blockedQueue;
        std::deque<std::pair<CobTime, CobThread*>> sleepingQueue;
        std::deque<CobThread*> finishedQueue;

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

        std::optional<CobThread> createNonScheduledThread(const std::string& functionName, const std::vector<int>& params);

        CobThread createNonScheduledThread(unsigned int functionId, const std::vector<int>& params);

        const CobThread* createThread(unsigned int functionId, const std::vector<int>& params, unsigned int signalMask);

        const CobThread* createThread(unsigned int functionId, const std::vector<int>& params);

        std::optional<const CobThread*> createThread(const std::string& functionName, const std::vector<int>& params);

        std::optional<const CobThread*> createThread(const std::string& functionName);

        void deleteThread(const CobThread* thread);

        /**
         * Sends a signal to all threads.
         * If the signal is non-zero after being ANDed
         * with the thread's signal mask, the thread is killed.
         */
        void sendSignal(unsigned int signal);

        /**
         * Attempts to collect the return value from a thread.
         * The return value will be available for collection
         * if the thread finished in the previous frame.
         * If the return value is not collected,
         * the cob execution service will clean up the thread
         * next frame.
         */
        std::optional<int> tryReapThread(const CobThread* thread);

        bool isNotCorrupt() const;

        /**
         * Pushes a value onto the first ready thread's stack.
         */
        void pushResult(int result);

    private:
        void removeThreadFromQueues(const CobThread* thread);

        bool isPresentInAQueue(const CobThread* thread) const;
    };
}
