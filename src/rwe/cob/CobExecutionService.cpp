#include "CobExecutionContext.h"
#include "CobExecutionService.h"

namespace rwe
{
    class BlockCheckVisitor : public boost::static_visitor<bool>
    {
    private:
        GameSimulation* simulation;
        CobEnvironment* env;
        UnitId unitId;

    public:
        BlockCheckVisitor(GameSimulation* simulation, CobEnvironment* env, UnitId unitId)
            : simulation(simulation), env(env), unitId(unitId)
        {
        }

        bool operator()(const CobEnvironment::BlockedStatus::Move& condition) const
        {
            const auto& pieceName = env->_script->pieces.at(condition.object);
            return !simulation->isPieceMoving(unitId, pieceName, condition.axis);
        }

        bool operator()(const CobEnvironment::BlockedStatus::Turn& condition) const
        {
            const auto& pieceName = env->_script->pieces.at(condition.object);
            return !simulation->isPieceTurning(unitId, pieceName, condition.axis);
        }

        bool operator()(const CobEnvironment::BlockedStatus::Sleep& condition) const
        {
            return simulation->gameTime >= condition.wakeUpTime;
        }
    };

    class ThreadRescheduleVisitor : public boost::static_visitor<>
    {
    private:
        CobEnvironment* const env;
        CobThread* const thread;

    public:
        ThreadRescheduleVisitor(CobEnvironment* env, CobThread* thread) : env(env), thread(thread)
        {
        }

        void operator()(const CobEnvironment::BlockedStatus& status) const
        {
            env->blockedQueue.emplace_back(status, thread);
        }
        void operator()(const CobEnvironment::FinishedStatus&) const
        {
            env->finishedQueue.emplace_back(thread);
        }
        void operator()(const CobEnvironment::SignalStatus& status) const
        {
            env->readyQueue.emplace_front(thread);
            env->sendSignal(status.signal);
        }
    };

    void CobExecutionService::run(GameSimulation& simulation, UnitId unitId)
    {
        auto& unit = simulation.getUnit(unitId);
        auto& env = *unit.cobEnvironment;

        // clean up any finished threads that were not reaped last frame
        for (const auto& thread : env.finishedQueue)
        {
            env.deleteThread(thread);
        }
        env.finishedQueue.clear();

        // check if any blocked threads can be unblocked
        // and move them back into the ready queue
        for (auto it = env.blockedQueue.begin(); it != env.blockedQueue.end();)
        {
            const auto& pair = *it;
            const auto& status = pair.first;

            auto isUnblocked = boost::apply_visitor(BlockCheckVisitor(&simulation, &env, unitId), status.condition);
            if (isUnblocked)
            {
                it = env.blockedQueue.erase(it);
                env.readyQueue.push_back(pair.second);
            }
            else
            {
                ++it;
            }
        }

        // execute ready threads
        while (!env.readyQueue.empty())
        {
            auto thread = env.readyQueue.front();
            env.readyQueue.pop_front();

            CobExecutionContext context(&simulation, &env, thread, unitId);

            auto status = context.execute();

            boost::apply_visitor(ThreadRescheduleVisitor(&env, thread), status);
        }
    }
}
