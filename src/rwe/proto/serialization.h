#pragma once

#include <network.pb.h>
#include <rwe/PlayerCommand.h>
#include <rwe/sim/UnitFireOrders.h>
#include <vector>

namespace rwe
{
    void serializeVector(const SimVector& v, proto::SimVector& out);

    proto::PlayerUnitCommand::IssueOrder::IssueKind serializeIssueKind(const PlayerUnitCommand::IssueOrder::IssueKind& kind);

    proto::PlayerUnitCommand::SetFireOrders::FireOrders serializeFireOrders(const UnitFireOrders& orders);

    void serializePlayerCommand(const PlayerCommand& command, proto::PlayerCommand& out);

    std::vector<PlayerCommand> deserializeCommandSet(const proto::GameUpdateMessage_PlayerCommandSet& set);

    PlayerCommand deserializeCommand(const proto::PlayerCommand& cmd);

    PlayerUnitCommand deserializeUnitCommand(const proto::PlayerUnitCommand& cmd);

    PlayerUnitCommand::IssueOrder deserializeIssueOrder(const proto::PlayerUnitCommand::IssueOrder& cmd);

    PlayerUnitCommand::IssueOrder::IssueKind deserializeIssueKind(const proto::PlayerUnitCommand::IssueOrder::IssueKind& kind);

    UnitFireOrders deserializeFireOrders(const proto::PlayerUnitCommand::SetFireOrders::FireOrders& orders);

    UnitOrder deserializeUnitOrder(const proto::PlayerUnitCommand::IssueOrder& cmd);

    SimVector deserializeVector(const proto::SimVector& v);
}
