import * as protocol from "../../game-server/protocol";

export interface DisconnectGameAction {
  type: "DISCONNECT_GAME";
}

export function disconnectGame(): DisconnectGameAction {
  return {
    type: "DISCONNECT_GAME",
  };
}

export interface ReceiveHandshakeResponseAction {
  type: "RECEIVE_HANDSHAKE_RESPONSE";
  payload: protocol.HandshakeResponsePayload;
}

export function receiveHandshakeResponse(
  payload: protocol.HandshakeResponsePayload
): ReceiveHandshakeResponseAction {
  return {
    type: "RECEIVE_HANDSHAKE_RESPONSE",
    payload,
  };
}

export interface ReceivePlayerJoinedAction {
  type: "RECEIVE_PLAYER_JOINED";
  payload: protocol.PlayerJoinedPayload;
}

export function receivePlayerJoined(
  payload: protocol.PlayerJoinedPayload
): ReceivePlayerJoinedAction {
  return {
    type: "RECEIVE_PLAYER_JOINED",
    payload,
  };
}

export interface ReceivePlayerLeftAction {
  type: "RECEIVE_PLAYER_LEFT";
  payload: protocol.PlayerLeftPayload;
}

export function receivePlayerLeft(
  payload: protocol.PlayerLeftPayload
): ReceivePlayerLeftAction {
  return {
    type: "RECEIVE_PLAYER_LEFT",
    payload,
  };
}

export interface ReceivePlayerReadyAction {
  type: "RECEIVE_PLAYER_READY";
  payload: protocol.PlayerReadyPayload;
}

export function receivePlayerReady(
  payload: protocol.PlayerReadyPayload
): ReceivePlayerReadyAction {
  return {
    type: "RECEIVE_PLAYER_READY",
    payload,
  };
}

export interface ReceivePlayerChangedSideAction {
  type: "RECEIVE_PLAYER_CHANGED_SIDE";
  payload: protocol.PlayerChangedSidePayload;
}

export function receivePlayerChangedSide(
  payload: protocol.PlayerChangedSidePayload
): ReceivePlayerChangedSideAction {
  return {
    type: "RECEIVE_PLAYER_CHANGED_SIDE",
    payload,
  };
}

export interface ReceivePlayerChangedTeamAction {
  type: "RECEIVE_PLAYER_CHANGED_TEAM";
  payload: protocol.PlayerChangedTeamPayload;
}

export function receivePlayerChangedTeam(
  payload: protocol.PlayerChangedTeamPayload
): ReceivePlayerChangedTeamAction {
  return {
    type: "RECEIVE_PLAYER_CHANGED_TEAM",
    payload,
  };
}

export interface ReceivePlayerChangedColorAction {
  type: "RECEIVE_PLAYER_CHANGED_COLOR";
  payload: protocol.PlayerChangedColorPayload;
}

export function receivePlayerChangedColor(
  payload: protocol.PlayerChangedColorPayload
): ReceivePlayerChangedColorAction {
  return {
    type: "RECEIVE_PLAYER_CHANGED_COLOR",
    payload,
  };
}

export interface ReceiveSlotOpenedAction {
  type: "RECEIVE_SLOT_OPENED";
  payload: protocol.SlotOpenedPayload;
}

export function receiveSlotOpened(
  payload: protocol.SlotOpenedPayload
): ReceiveSlotOpenedAction {
  return {
    type: "RECEIVE_SLOT_OPENED",
    payload,
  };
}

export interface ReceiveSlotClosedAction {
  type: "RECEIVE_SLOT_CLOSED";
  payload: protocol.SlotClosedPayload;
}

export function receiveSlotClosed(
  payload: protocol.SlotClosedPayload
): ReceiveSlotClosedAction {
  return {
    type: "RECEIVE_SLOT_CLOSED",
    payload,
  };
}

export interface ReceiveChatMessageAction {
  type: "RECEIVE_CHAT_MESSAGE";
  payload: protocol.PlayerChatMessagePayload;
}

export function receiveChatMessage(
  payload: protocol.PlayerChatMessagePayload
): ReceiveChatMessageAction {
  return {
    type: "RECEIVE_CHAT_MESSAGE",
    payload,
  };
}

export interface ReceiveMapChangedAction {
  type: "RECEIVE_MAP_CHANGED";
  data: protocol.MapChangedPayload;
}

export function receiveMapChanged(
  data: protocol.MapChangedPayload
): ReceiveMapChangedAction {
  return {
    type: "RECEIVE_MAP_CHANGED",
    data,
  };
}

export interface ReceiveStartGameAction {
  type: "RECEIVE_START_GAME";
  payload: protocol.StartGamePayload;
}

export function receiveStartGame(
  payload: protocol.StartGamePayload
): ReceiveStartGameAction {
  return {
    type: "RECEIVE_START_GAME",
    payload,
  };
}

export interface ReceiveActiveModsChangedAction {
  type: "RECEIVE_ACTIVE_MODS_CHANGED";
  payload: protocol.ActiveModsChangedPayload;
}

export function receiveActiveModsChanged(
  payload: protocol.ActiveModsChangedPayload
): ReceiveActiveModsChangedAction {
  return {
    type: "RECEIVE_ACTIVE_MODS_CHANGED",
    payload,
  };
}

export type GameClientAction =
  | ReceiveHandshakeResponseAction
  | ReceivePlayerJoinedAction
  | ReceivePlayerLeftAction
  | ReceiveChatMessageAction
  | ReceivePlayerChangedSideAction
  | ReceivePlayerChangedTeamAction
  | ReceivePlayerChangedColorAction
  | ReceivePlayerReadyAction
  | ReceiveSlotOpenedAction
  | ReceiveSlotClosedAction
  | ReceiveStartGameAction
  | DisconnectGameAction
  | ReceiveMapChangedAction
  | ReceiveActiveModsChangedAction;
