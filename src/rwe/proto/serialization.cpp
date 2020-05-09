#include "serialization.h"


namespace rwe
{
    class WriteAttackTargetVisitor
    {
    private:
        proto::AttackOrder* cmd;

    public:
        explicit WriteAttackTargetVisitor(proto::AttackOrder& cmd) : cmd(&cmd) {}

        void operator()(const SimVector& v)
        {
            auto& out = *cmd->mutable_ground();
            serializeVector(v, out);
        }

        void operator()(const UnitId& u)
        {
            cmd->set_unit(u.value);
        }
    };

    class WriteUnitOrderVisitor
    {
    private:
        proto::PlayerUnitCommand::IssueOrder* cmd;

    public:
        explicit WriteUnitOrderVisitor(proto::PlayerUnitCommand::IssueOrder& cmd) : cmd(&cmd) {}

        void operator()(const MoveOrder& o)
        {
            auto& out = *cmd->mutable_move();
            auto& dest = *out.mutable_destination();
            serializeVector(o.destination, dest);
        }

        void operator()(const AttackOrder& o)
        {
            auto& out = *cmd->mutable_attack();
            WriteAttackTargetVisitor visitor(out);
            std::visit(visitor, o.target);
        }

        void operator()(const BuildOrder& o)
        {
            auto& out = *cmd->mutable_build();
            out.set_unit_type(o.unitType);
            auto& pos = *out.mutable_position();
            serializeVector(o.position, pos);
        }

        void operator()(const BuggerOffOrder&)
        {
            throw std::logic_error("Cannot serialize BuggerOffOrder");
        }

        void operator()(const CompleteBuildOrder& o)
        {
            auto& out = *cmd->mutable_complete_build();
            out.set_unit(o.target.value);
        }

        void operator()(const GuardOrder& o)
        {
            auto& out = *cmd->mutable_guard();
            out.set_unit(o.target.value);
        }
    };

    class WriteUnitCommandVisitor
    {
    private:
        proto::PlayerUnitCommand* cmd;

    public:
        explicit WriteUnitCommandVisitor(proto::PlayerUnitCommand& cmd) : cmd(&cmd) {}

    public:
        void operator()(const PlayerUnitCommand::IssueOrder& c)
        {
            auto& out = *cmd->mutable_order();
            out.set_issue_kind(serializeIssueKind(c.issueKind));
            WriteUnitOrderVisitor visitor(out);
            std::visit(visitor, c.order);
        }

        void operator()(const PlayerUnitCommand::ModifyBuildQueue& c)
        {
            auto& out = *cmd->mutable_modify_build_queue();
            out.set_count(c.count);
            out.set_unit_type(c.unitType);
        }

        void operator()(const PlayerUnitCommand::Stop&)
        {
            cmd->mutable_stop();
        }

        void operator()(const PlayerUnitCommand::SetFireOrders& c)
        {
            auto& out = *cmd->mutable_set_fire_orders();
            out.set_orders(serializeFireOrders(c.orders));
        }

        void operator()(const PlayerUnitCommand::SetOnOff& c)
        {
            auto& out = *cmd->mutable_set_on_off();
            out.set_ison(c.on);
        }
    };

    class WritePlayerCommandVisitor
    {
    private:
        proto::PlayerCommand* cmd;

    public:
        explicit WritePlayerCommandVisitor(proto::PlayerCommand& cmd) : cmd(&cmd) {}

    public:
        void operator()(const PlayerUnitCommand& c)
        {
            auto& out = *cmd->mutable_unit_command();
            out.set_unit(c.unit.value);
            WriteUnitCommandVisitor visitor(out);
            std::visit(visitor, c.command);
        }

        void operator()(const PlayerPauseGameCommand&)
        {
            cmd->mutable_pause();
        }

        void operator()(const PlayerUnpauseGameCommand&)
        {
            cmd->mutable_unpause();
        }
    };

    void serializeVector(const SimVector& v, proto::SimVector& out)
    {
        out.set_x(simScalarToFloat(v.x));
        out.set_y(simScalarToFloat(v.y));
        out.set_z(simScalarToFloat(v.z));
    }

    proto::PlayerUnitCommand::IssueOrder::IssueKind
    serializeIssueKind(const PlayerUnitCommand::IssueOrder::IssueKind& kind)
    {
        switch (kind)
        {
            case PlayerUnitCommand::IssueOrder::IssueKind::Immediate:
                return proto::PlayerUnitCommand::IssueOrder::Immediate;
            case PlayerUnitCommand::IssueOrder::IssueKind::Queued:
                return proto::PlayerUnitCommand::IssueOrder::Queued;
            default:
                throw std::logic_error("Invalid IssueKind value");
        }
    }

    proto::PlayerUnitCommand::SetFireOrders::FireOrders serializeFireOrders(const UnitFireOrders& orders)
    {
        switch (orders)
        {
            case UnitFireOrders::HoldFire:
                return proto::PlayerUnitCommand::SetFireOrders::HoldFire;
            case UnitFireOrders::ReturnFire:
                return proto::PlayerUnitCommand::SetFireOrders::ReturnFire;
            case UnitFireOrders::FireAtWill:
                return proto::PlayerUnitCommand::SetFireOrders::FireAtWill;
            default:
                throw std::logic_error("Invalid UnitFireOrders value");
        }
    }

    void serializePlayerCommand(const PlayerCommand& command, proto::PlayerCommand& out)
    {
        WritePlayerCommandVisitor visitor(out);
        std::visit(visitor, command);
    }

    std::vector<PlayerCommand> deserializeCommandSet(const proto::GameUpdateMessage_PlayerCommandSet& set)
    {
        std::vector<PlayerCommand> out;

        for (int i = 0; i < set.command_size(); ++i)
        {
            out.push_back(deserializeCommand(set.command(i)));
        }

        return out;
    }

    PlayerCommand deserializeCommand(const proto::PlayerCommand& cmd)
    {
        if (cmd.has_pause())
        {
            return PlayerPauseGameCommand();
        }

        if (cmd.has_unpause())
        {
            return PlayerUnpauseGameCommand();
        }

        if (cmd.has_unit_command())
        {
            return deserializeUnitCommand(cmd.unit_command());
        }

        throw std::runtime_error("Failed to deserialize command");
    }

    PlayerUnitCommand deserializeUnitCommand(const proto::PlayerUnitCommand& cmd)
    {
        if (cmd.has_order())
        {
            return PlayerUnitCommand(UnitId(cmd.unit()), deserializeIssueOrder(cmd.order()));
        }

        if (cmd.has_modify_build_queue())
        {
            const auto& mod = cmd.modify_build_queue();
            return PlayerUnitCommand(UnitId(cmd.unit()), PlayerUnitCommand::ModifyBuildQueue{mod.count(), mod.unit_type()});
        }

        if (cmd.has_stop())
        {
            return PlayerUnitCommand(UnitId(cmd.unit()), PlayerUnitCommand::Stop());
        }

        if (cmd.has_set_fire_orders())
        {
            return PlayerUnitCommand(UnitId(cmd.unit()), PlayerUnitCommand::SetFireOrders{deserializeFireOrders(cmd.set_fire_orders().orders())});
        }

        if (cmd.has_set_on_off())
        {
            return PlayerUnitCommand(UnitId(cmd.unit()), PlayerUnitCommand::SetOnOff{cmd.set_on_off().ison()});
        }

        throw std::runtime_error("Failed to deserialize unit command");
    }

    PlayerUnitCommand::IssueOrder deserializeIssueOrder(const proto::PlayerUnitCommand::IssueOrder& cmd)
    {
        return PlayerUnitCommand::IssueOrder(deserializeUnitOrder(cmd), deserializeIssueKind(cmd.issue_kind()));
    }

    PlayerUnitCommand::IssueOrder::IssueKind
    deserializeIssueKind(const proto::PlayerUnitCommand::IssueOrder::IssueKind& kind)
    {
        switch (kind)
        {
            case proto::PlayerUnitCommand::IssueOrder::Immediate:
                return PlayerUnitCommand::IssueOrder::IssueKind::Immediate;
            case proto::PlayerUnitCommand::IssueOrder::Queued:
                return PlayerUnitCommand::IssueOrder::IssueKind::Queued;
            default:
                throw std::runtime_error("Failed to deserialize issue kind");
        }
    }

    UnitFireOrders deserializeFireOrders(const proto::PlayerUnitCommand::SetFireOrders::FireOrders& orders)
    {
        switch (orders)
        {
            case proto::PlayerUnitCommand::SetFireOrders::HoldFire:
                return UnitFireOrders::HoldFire;
            case proto::PlayerUnitCommand::SetFireOrders::ReturnFire:
                return UnitFireOrders::ReturnFire;
            case proto::PlayerUnitCommand::SetFireOrders::FireAtWill:
                return UnitFireOrders::FireAtWill;
            default:
                throw std::runtime_error("Failed to deserialize fire orders");
        }
    }

    UnitOrder deserializeUnitOrder(const proto::PlayerUnitCommand::IssueOrder& cmd)
    {
        if (cmd.has_move())
        {
            const auto& move = cmd.move();
            return MoveOrder(deserializeVector(move.destination()));
        }

        if (cmd.has_attack())
        {
            const auto& attack = cmd.attack();
            if (attack.has_unit())
            {
                return AttackOrder(UnitId(attack.unit()));
            }

            if (attack.has_ground())
            {
                return AttackOrder(deserializeVector(attack.ground()));
            }

            throw std::runtime_error("Failed to deserlialize attack order");
        }

        if (cmd.has_build())
        {
            const auto& build = cmd.build();
            return BuildOrder(build.unit_type(), deserializeVector(build.position()));
        }

        if (cmd.has_complete_build())
        {
            const auto& build = cmd.complete_build();
            return CompleteBuildOrder(UnitId(build.unit()));
        }

        if (cmd.has_guard())
        {
            const auto& guard = cmd.guard();
            return GuardOrder(UnitId(guard.unit()));
        }

        throw std::runtime_error("Failed to deserlialize unit order");
    }

    SimVector deserializeVector(const proto::SimVector& v)
    {
        return SimVector(SimScalar(v.x()), SimScalar(v.y()), SimScalar(v.z()));
    }
}
