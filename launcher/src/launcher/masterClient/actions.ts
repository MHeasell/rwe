import {
  GameCreatedEventPayload,
  GameDeletedEventPayload,
  GameUpdatedEventPayload,
  GetGamesResponsePayload,
  CreateGameResponsePayload,
} from "../../master-server/protocol";

export interface MasterServerConnectAction {
  type: "MASTER_SERVER_CONNECT";
}

export function masterServerConnect(): MasterServerConnectAction {
  return {
    type: "MASTER_SERVER_CONNECT",
  };
}

export interface MasterServerDisconnectAction {
  type: "MASTER_SERVER_DISCONNECT";
}

export function masterServerDisconnect(): MasterServerDisconnectAction {
  return {
    type: "MASTER_SERVER_DISCONNECT",
  };
}

export interface ReceiveRoomsAction {
  type: "RECEIVE_ROOMS";
  rooms: GetGamesResponsePayload;
}

export function receiveRooms(
  rooms: GetGamesResponsePayload
): ReceiveRoomsAction {
  return {
    type: "RECEIVE_ROOMS",
    rooms,
  };
}

export interface ReceiveGameCreatedAction {
  type: "RECEIVE_GAME_CREATED";
  payload: GameCreatedEventPayload;
}

export function receiveGameCreated(
  payload: GameCreatedEventPayload
): ReceiveGameCreatedAction {
  return {
    type: "RECEIVE_GAME_CREATED",
    payload,
  };
}

export interface ReceiveGameUpdatedAction {
  type: "RECEIVE_GAME_UPDATED";
  payload: GameUpdatedEventPayload;
}

export function receiveGameUpdated(
  payload: GameUpdatedEventPayload
): ReceiveGameUpdatedAction {
  return {
    type: "RECEIVE_GAME_UPDATED",
    payload,
  };
}

export interface ReceiveGameDeletedAction {
  type: "RECEIVE_GAME_DELETED";
  payload: GameDeletedEventPayload;
}

export function receiveGameDeleted(
  payload: GameDeletedEventPayload
): ReceiveGameDeletedAction {
  return {
    type: "RECEIVE_GAME_DELETED",
    payload,
  };
}

export interface ReceiveCreateGameResponseAction {
  type: "RECEIVE_CREATE_GAME_RESPONSE";
  payload: CreateGameResponsePayload;
}

export function receiveCreateGameResponse(
  payload: CreateGameResponsePayload
): ReceiveCreateGameResponseAction {
  return {
    type: "RECEIVE_CREATE_GAME_RESPONSE",
    payload,
  };
}

export type MasterClientAction =
  | MasterServerConnectAction
  | MasterServerDisconnectAction
  | ReceiveRoomsAction
  | ReceiveGameCreatedAction
  | ReceiveGameUpdatedAction
  | ReceiveGameDeletedAction
  | ReceiveCreateGameResponseAction;
