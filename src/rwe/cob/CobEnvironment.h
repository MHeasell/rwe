#ifndef RWE_COBENVIRONMENT_H
#define RWE_COBENVIRONMENT_H

#include <vector>
#include <memory>
#include <rwe/Cob.h>
#include <rwe/cob/CobThread.h>

namespace rwe
{
    class GameScene;

    class CobEnvironment
    {
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

        void createThread(unsigned int functionId, const std::vector<int>& params);

        void createThread(const std::string& functionName, const std::vector<int>& params);

        void executeThreads();

    private:
        void deleteThread(const CobThread* thread);
    };
}

#endif
