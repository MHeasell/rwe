#include "serialization.h"
#include <stdexcept>

namespace rwe
{
    class WriteAttackTargetVisitor : public boost::static_visitor<>
    {
    private:
        proto::AttackOrder* cmd;

    public:
        explicit WriteAttackTargetVisitor(proto::AttackOrder& cmd) : cmd(&cmd) {}

        void operator()(const Vector3f& v)
        {
            auto& out = *cmd->mutable_ground();
            serializeVector(v, out);
        }

        void operator()(const UnitId& u)
        {
            cmd->set_unit(u.value);
        }
    };

    class WriteUnitOrderVisitor : public boost::static_visitor<>
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
            boost::apply_visitor(visitor, o.target);
        }
    };

    class WriteUnitCommandVisitor : public boost::static_visitor<>
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
            boost::apply_visitor(visitor, c.order);
        }

        void operator()(const PlayerUnitCommand::Stop&)
        {
            cmd->mutable_stop();
        }
    };

    class WritePlayerCommandVisitor : public boost::static_visitor<>
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
            boost::apply_visitor(visitor, c.command);
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

    void serializeVector(const Vector3f& v, proto::Vector3f& out)
    {
        out.set_x(v.x);
        out.set_y(v.y);
        out.set_z(v.z);
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

    void serializePlayerCommand(const PlayerCommand& command, proto::PlayerCommand& out)
    {
        WritePlayerCommandVisitor visitor(out);
        boost::apply_visitor(visitor, command);
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

        if (cmd.has_stop())
        {
            return PlayerUnitCommand(UnitId(cmd.unit()), PlayerUnitCommand::Stop());
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

        throw std::runtime_error("Failed to deserlialize unit order");
    }

    Vector3f deserializeVector(const proto::Vector3f& v)
    {
        return Vector3f(v.x(), v.y(), v.z());
    }
}
