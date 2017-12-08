#include "CobExecutionService.h"
#include "CobExecutionContext.h"

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
            env->deleteThread(thread);
        }
    };

    void CobExecutionService::run(GameSimulation& simulation, UnitId unitId)
    {
        auto& unit = simulation.getUnit(unitId);
        auto& env = *unit.cobEnvironment;

        // check if any blocked threads can be unblocked
        std::vector<CobThread*> tempQueue;
        for (const auto& pair : env.blockedQueue)
        {
            const auto& status = boost::get<CobEnvironment::BlockedStatus>(pair.first);

            auto isUnblocked = boost::apply_visitor(BlockCheckVisitor(&simulation, &env, unitId), status.condition);
            if (isUnblocked)
            {
                tempQueue.push_back(pair.second);
            }
        }

        // move unblocked threads back into the ready queue
        for (const auto& t : tempQueue)
        {
            auto it = std::find_if(env.blockedQueue.begin(), env.blockedQueue.end(), [&t](const auto& p) { return p.second == t; });
            env.blockedQueue.erase(it);
            env.readyQueue.push_back(t);
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
