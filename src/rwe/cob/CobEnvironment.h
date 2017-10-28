#ifndef RWE_COBENVIRONMENT_H
#define RWE_COBENVIRONMENT_H

#include <vector>
#include <memory>
#include <rwe/Cob.h>
#include <rwe/cob/CobThread.h>
#include <boost/variant.hpp>


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

    private:
        GameScene* scene;
        const CobScript* _script;

        unsigned int unitId;
        std::vector<int> _statics;

        std::vector<std::unique_ptr<CobThread>> threads;

        std::deque<CobThread*> readyQueue;
        std::deque<CobThread*> blockedQueue;

    public:
        CobEnvironment(GameScene* scene, const CobScript* _script, unsigned int unitId);

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

        void createThread(unsigned int functionId, const std::vector<int>& params);

        void createThread(const std::string& functionName, const std::vector<int>& params);

        void executeThreads();

    private:
        void deleteThread(const CobThread* thread);
    };
}

#endif
