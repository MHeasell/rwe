#ifndef RWE_UNIT_H
#define RWE_UNIT_H

#include <rwe/UnitMesh.h>
#include "GraphicsContext.h"
#include <memory>
#include <rwe/cob/CobEnvironment.h>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <rwe/geometry/BoundingBox3f.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/AudioService.h>
#include <deque>

namespace rwe
{
    struct MoveOrder
    {
        Vector3f destination;
        explicit MoveOrder(const Vector3f& destination);
    };

    using UnitOrder = boost::variant<MoveOrder>;

    UnitOrder createMoveOrder(const Vector3f& destination);

    class Unit
    {
    public:
        UnitMesh mesh;
        Vector3f position;
        std::unique_ptr<CobEnvironment> cobEnvironment;
        SelectionMesh selectionMesh;
        boost::optional<AudioService::SoundHandle> selectionSound;
        boost::optional<AudioService::SoundHandle> okSound;
        boost::optional<AudioService::SoundHandle> arrivedSound;
        unsigned int owner;

        /**
         * Anticlockwise rotation of the unit around the Y axis in radians.
         * The other two axes of rotation are normally determined
         * by the normal of the terrain the unit is standing on.
         */
        float rotation{0.0f};


        /**
         * Rate at which the unit turns in rads/tick.
         */
        float turnRate;

        /**
         * Rate at which the unit is travelling forwards in game units/tick.
         */
        float currentSpeed{0.0f};

        /**
         * Maximum speed the unit can travel forwards in game units/tick.
         */
        float maxSpeed;

        /**
         * Speed at which the unit accelerates in game units/tick.
         */
        float acceleration;

        /**
         * Speed at which the unit brakes in game units/tick.
         */
        float brakeRate;

        std::deque<UnitOrder> orders;

        Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment, SelectionMesh&& selectionMesh);

        void moveObject(const std::string& pieceName, Axis axis, float targetPosition, float speed);

        void moveObjectNow(const std::string& pieceName, Axis axis, float targetPosition);

        void turnObject(const std::string& pieceName, Axis axis, float targetAngle, float speed);

        void turnObjectNow(const std::string& pieceName, Axis axis, float targetAngle);

        bool isMoveInProgress(const std::string& pieceName, Axis axis) const;

        bool isTurnInProgress(const std::string& pieceName, Axis axis) const;

        /**
         * Returns a value if the given ray intersects this unit
         * for the purposes of unit selection.
         * The value returned is the distance along the ray
         * where the intersection occurred.
         */
        boost::optional<float> selectionIntersect(const Ray3f& ray) const;

        void render(
            GraphicsContext& context,
            ShaderProgramIdentifier textureShader,
            ShaderProgramIdentifier colorShader,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix) const;

        void renderSelectionRect(
            GraphicsContext& context,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix,
            ShaderProgramIdentifier shader) const;

        bool isOwnedBy(unsigned int playerId) const;

        void clearOrders();

        void addOrder(const UnitOrder& order);

        void update(float dt);
    };
}

#endif
