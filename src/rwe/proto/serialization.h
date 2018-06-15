#ifndef RWE_SERIALIZATION_H
#define RWE_SERIALIZATION_H

#include <network.pb.h>
#include <rwe/PlayerCommand.h>
#include <vector>

namespace rwe
{
    void serializeVector(const Vector3f& v, proto::Vector3f& out);

    proto::PlayerUnitCommand::IssueOrder::IssueKind serializeIssueKind(const PlayerUnitCommand::IssueOrder::IssueKind& kind);

    void serializePlayerCommand(const PlayerCommand& command, proto::PlayerCommand& out);

    std::vector<PlayerCommand> deserializeCommandSet(const proto::GameUpdateMessage_PlayerCommandSet& set);

    PlayerCommand deserializeCommand(const proto::PlayerCommand& cmd);

    PlayerUnitCommand deserializeUnitCommand(const proto::PlayerUnitCommand& cmd);

    PlayerUnitCommand::IssueOrder deserializeIssueOrder(const proto::PlayerUnitCommand::IssueOrder& cmd);

    PlayerUnitCommand::IssueOrder::IssueKind deserializeIssueKind(const proto::PlayerUnitCommand::IssueOrder::IssueKind& kind);

    UnitOrder deserializeUnitOrder(const proto::PlayerUnitCommand::IssueOrder& cmd);

    Vector3f deserializeVector(const proto::Vector3f& v);
}

#endif
