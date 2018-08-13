import { execFile } from "child_process";
import * as path from "path";
import { Dispatch } from "redux";
import { ThunkAction } from "redux-thunk";
import { State } from "./reducers";
import { GetRoomsResponse } from "./web";
import { PlayerJoinedPayload, HandshakeResponsePayload, PlayerLeftPayload, PlayerReadyPayload } from "./game-server";

export interface SelectGameAction {
  type: "SELECT_GAME";
  gameId: number;
}

export function selectGame(gameId: number): SelectGameAction {
  return {
    type: "SELECT_GAME",
    gameId
  };
}

export interface JoinSelectedGameAction {
  type: "JOIN_SELECTED_GAME";
}

export function joinSelectedGame(): JoinSelectedGameAction {
  return {
    type: "JOIN_SELECTED_GAME",
  };
}

export interface JoinSelectedGameConfirmAction {
  type: "JOIN_SELECTED_GAME_CONFIRM";
  name: string;
}

export function joinSelectedGameConfirm(name: string): JoinSelectedGameConfirmAction {
  return {
    type: "JOIN_SELECTED_GAME_CONFIRM",
    name,
  };
}

export interface JoinSelectedGameCancelAction {
  type: "JOIN_SELECTED_GAME_CANCEL";
}

export function joinSelectedGameCancel(): JoinSelectedGameCancelAction {
  return {
    type: "JOIN_SELECTED_GAME_CANCEL",
  };
}

export interface HostGameAction {
  type: "HOST_GAME";
}

export function hostGame(): HostGameAction {
  return {
    type: "HOST_GAME",
  };
}

export interface HostGameFormCancelAction {
  type: "HOST_GAME_FORM_CANCEL";
}

export function hostGameFormCancel(): HostGameFormCancelAction {
  return {
    type: "HOST_GAME_FORM_CANCEL",
  };
}

export interface HostGameFormConfirmAction {
  type: "HOST_GAME_FORM_CONFIRM";
  playerName: string;
  gameDescription: string;
  players: number;
}

export function hostGameFormConfirm(playerName: string, gameDescription: string, players: number): HostGameFormConfirmAction {
  return {
    type: "HOST_GAME_FORM_CONFIRM",
    playerName,
    gameDescription,
    players
  };
}

export interface LaunchRweBeginAction {
  type: "LAUNCH_RWE_BEGIN";
}

export function launchRweBegin(): LaunchRweBeginAction {
  return {
    type: "LAUNCH_RWE_BEGIN",
  };
}

export interface LaunchRweEndAction {
  type: "LAUNCH_RWE_END";
}

export function launchRweEnd(): LaunchRweEndAction {
  return {
    type: "LAUNCH_RWE_END",
  };
}

export interface ReceiveRoomsAction {
  type: "RECEIVE_ROOMS";
  rooms: GetRoomsResponse;
}

export function receiveRooms(rooms: GetRoomsResponse): ReceiveRoomsAction {
  return {
    type: "RECEIVE_ROOMS",
    rooms,
  };
}

export interface ReceiveChatMessageAction {
  type: "RECEIVE_CHAT_MESSAGE";
  message: string;
}

export function receiveChatMessage(message: string): ReceiveChatMessageAction {
  return {
    type: "RECEIVE_CHAT_MESSAGE",
    message,
  };
}

export interface SendChatMessageAction {
  type: "SEND_CHAT_MESSAGE";
  message: string;
}

export function sendChatMessage(message: string): SendChatMessageAction {
  return {
    type: "SEND_CHAT_MESSAGE",
    message,
  };
}

export interface ReceivePlayerJoinedAction {
  type: "RECEIVE_PLAYER_JOINED";
  payload: PlayerJoinedPayload;
}

export function receivePlayerJoined(payload: PlayerJoinedPayload): ReceivePlayerJoinedAction {
  return {
    type: "RECEIVE_PLAYER_JOINED",
    payload,
  };
}

export interface ReceivePlayerLeftAction {
  type: "RECEIVE_PLAYER_LEFT";
  payload: PlayerLeftPayload;
}

export function receivePlayerLeft(payload: PlayerLeftPayload): ReceivePlayerLeftAction {
  return {
    type: "RECEIVE_PLAYER_LEFT",
    payload,
  };
}

export interface ReceiveHandshakeResponseAction {
  type: "RECEIVE_HANDSHAKE_RESPONSE";
  payload: HandshakeResponsePayload;
}

export function receiveHandshakeResponse(payload: HandshakeResponsePayload): ReceiveHandshakeResponseAction {
  return {
    type: "RECEIVE_HANDSHAKE_RESPONSE",
    payload,
  };
}

export interface ReceivePlayerReadyAction {
  type: "RECEIVE_PLAYER_READY";
  payload: PlayerReadyPayload;
}

export function receivePlayerReady(payload: PlayerReadyPayload): ReceivePlayerReadyAction {
  return {
    type: "RECEIVE_PLAYER_READY",
    payload,
  };
}

export interface LeaveGameAction {
  type: "LEAVE_GAME";
}

export function leaveGame(): LeaveGameAction {
  return {
    type: "LEAVE_GAME",
  };
}

export interface DisconnectGameAction {
  type: "DISCONNECT_GAME";
}

export function disconnectGame(): DisconnectGameAction {
  return {
    type: "DISCONNECT_GAME",
  };
}

export interface ToggleReadyAction {
  type: "TOGGLE_READY";
}

export function toggleReady(): ToggleReadyAction {
  return {
    type: "TOGGLE_READY",
  };
}

export type AppAction =
  | SelectGameAction
  | JoinSelectedGameAction
  | JoinSelectedGameConfirmAction
  | JoinSelectedGameCancelAction
  | HostGameAction
  | HostGameFormCancelAction
  | HostGameFormConfirmAction
  | LaunchRweBeginAction
  | LaunchRweEndAction
  | ReceiveRoomsAction
  | ReceiveHandshakeResponseAction
  | ReceivePlayerJoinedAction
  | ReceivePlayerLeftAction
  | ReceiveChatMessageAction
  | ReceivePlayerReadyAction
  | SendChatMessageAction
  | ToggleReadyAction
  | LeaveGameAction
  | DisconnectGameAction;

export function launchRwe(): ThunkAction<void, State, void, AppAction>  {
  return (dispatch: Dispatch, getState: () => State) => {
    if (getState().isRweRunning) {
      return;
    }

    const rweHome = process.env["RWE_HOME"];
    if (!rweHome) {
      console.error("Cannot launch RWE, RWE_HOME is not defined");
      return;
    }

    dispatch(launchRweBegin());
    // FIXME: assumes windows
    execFile(path.join(rweHome, "rwe.exe"), { cwd: rweHome }, (error: (null | Error), stdout: any, stderr: any) => {
      if (error) {
        console.error("RWE exited with code: " + (error as any).code);
        console.error("Message: " + error.message);
      }
      dispatch(launchRweEnd());
    });
  };
}
