#ifndef RWE_COBENVIRONMENT_H
#define RWE_COBENVIRONMENT_H

#include <vector>
#include <memory>
#include <rwe/Cob.h>
#include <rwe/cob/CobThread.h>
#include <boost/variant.hpp>
#include <rwe/UnitId.h>


namespace rwe
{
    class GameScene;

    class CobEnvironment
    {
    private:
        class ThreadRescheduleVisitor : public boost::static_visitor<>
        {
        private:
            CobEnvironment* env;
            CobThread* thread;

        public:
            ThreadRescheduleVisitor(CobEnvironment* env, CobThread* thread) : env(env), thread(thread)
            {
            }

            void operator()(const CobThread::ReadyStatus&) const
            {
                env->readyQueue.push_back(thread);
            }

            void operator()(const CobThread::BlockedStatus&) const
            {
                env->blockedQueue.push_back(thread);
            }
            void operator()(const CobThread::FinishedStatus&) const
            {
                env->deleteThread(thread);
            }
        };

        class BlockCheckVisitor : public boost::static_visitor<bool>
        {
        private:
            CobEnvironment* env;

        public:
            explicit BlockCheckVisitor(CobEnvironment* env) : env(env)
            {
            }

            bool operator()(const CobThread::BlockedStatus::Move& condition) const;

            bool operator()(const CobThread::BlockedStatus::Turn& condition) const;

            bool operator()(const CobThread::BlockedStatus::Sleep& condition) const;
        };

    private:
        GameScene* scene;
        const CobScript* _script;

        UnitId unitId;
        std::vector<int> _statics;

        std::vector<std::unique_ptr<CobThread>> threads;

        std::deque<CobThread*> readyQueue;
        std::deque<CobThread*> blockedQueue;

    public:
        CobEnvironment(GameScene* scene, const CobScript* _script, UnitId unitId);

        CobEnvironment(const CobEnvironment& other) = delete;
        CobEnvironment& operator=(const CobEnvironment& other) = delete;
        CobEnvironment(CobEnvironment&& other) = delete;
        CobEnvironment& operator=(CobEnvironment&& other) = delete;

    public:
        int getStatic(unsigned int id)
        {
            return _statics.at(id);
        }

        void setStatic(unsigned int id, int value)
        {
            _statics.at(id) = value;
        }

        const CobScript* script()
        {
            return _script;
        }

        void showObject(unsigned int objectId);

        void hideObject(unsigned int objectId);

        void moveObject(unsigned int objectId, Axis axis, float position, float speed);

        void moveObjectNow(unsigned int objectId, Axis axis, float position);

        void turnObject(unsigned int objectId, Axis axis, float angle, float speed);

        void turnObjectNow(unsigned int objectId, Axis axis, float angle);

        void createThread(unsigned int functionId, const std::vector<int>& params);

        void createThread(const std::string& functionName, const std::vector<int>& params);

        void createThread(const std::string& functionName);

        unsigned int getGameTime() const;

        void executeThreads();

    private:
        void deleteThread(const CobThread* thread);
    };
}

#endif
