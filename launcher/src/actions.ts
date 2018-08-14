import { execFile } from "child_process";
import * as path from "path";
import * as util from "util";
import { Dispatch } from "redux";
import { ThunkAction } from "redux-thunk";
import { State, GameRoom } from "./reducers";
import { GetRoomsResponse } from "./web";
import { PlayerJoinedPayload, HandshakeResponsePayload, PlayerLeftPayload, PlayerReadyPayload, PlayerChatMessagePayload } from "./game-server";
import { resolve } from "dns";

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
  payload: PlayerChatMessagePayload;
}

export function receiveChatMessage(payload: PlayerChatMessagePayload): ReceiveChatMessageAction {
  return {
    type: "RECEIVE_CHAT_MESSAGE",
    payload,
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

export interface StartGameAction {
  type: "START_GAME";
}

export function startGame(): StartGameAction {
  return {
    type: "START_GAME",
  };
}

export interface GameEndedAction {
  type: "GAME_ENDED";
}

export function gameEnded(): GameEndedAction {
  return {
    type: "GAME_ENDED",
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
  | DisconnectGameAction
  | StartGameAction
  | GameEndedAction;

interface RweArgsPlayerHuman {
  type: "human";
}

interface RweArgsPlayerComputer {
  type: "computer";
}

interface RweArgsPlayerRemote {
  type: "remote";
  host: string;
  port: number;
}

type RweArgsPlayerController = RweArgsPlayerHuman | RweArgsPlayerComputer | RweArgsPlayerRemote;

interface RweArgsPlayerInfo {
  side: "ARM" | "CORE";
  color: number;
  controller: RweArgsPlayerController;
}

interface RweArgs {
  dataPath?: string;
  map?: string;
  interface?: string;
  port?: number;
  players: RweArgsPlayerInfo[];
}

function serializeRweController(controller: RweArgsPlayerController): string {
  switch (controller.type) {
    case "human":
      return "Human";
    case "computer":
      return "Computer";
    case "remote":
      return `Network,${controller.host}:${controller.port.toString()}`;
    default:
      throw new Error("unknown controller type");
  }
}

function serializeRweArgs(args: RweArgs): string[] {
  const out = [];
  if (args.dataPath !== undefined) {
    out.push("--data-path", args.dataPath);
  }
  if (args.map !== undefined) {
    out.push("--map", args.map);
  }
  if (args.interface !== undefined) {
    out.push("--interface", args.interface);
  }
  if (args.port !== undefined) {
    out.push("--port", args.port.toString());
  }
  for (const p of args.players) {
    const controllerString = serializeRweController(p.controller);
    out.push("--player", `${controllerString};${p.side};${p.color}`);
  }
  return out;
}

// Naively quotes args, as if you were going to pass them through a shell,
// for display purposes.
// Don't use this for actual shell escaping, it's probably really insecure.
function quoteArg(arg: string) {
  if (arg.match(/[ "'\\]/)) {
    const escapedArg = arg.replace(/["\\]/, "\\$1");
    return `"${escapedArg}"`;
  }
  return arg;
}

function execRwe(args?: RweArgs): Promise<any> {
  const rweHome = process.env["RWE_HOME"];
  if (!rweHome) {
    return Promise.reject("Cannot launch RWE, RWE_HOME is not defined");
  }

  const serializedArgs = args ? serializeRweArgs(args) : undefined;

  return new Promise((resolve, reject) => {
    if (serializedArgs) {
      console.log("Launching RWE with args: " + serializedArgs.map(quoteArg).join(" "));
    }
    else {
      console.log("Launching RWE");
    }
    // FIXME: assumes windows
    execFile(path.join(rweHome, "rwe.exe"), serializedArgs, { cwd: rweHome }, (error: (null | Error), stdout: any, stderr: any) => {
      if (error) {
        const exitCode = (error as any).code;
        reject(`RWE exited with exit code ${exitCode}: ${error.message}`);
        return;
      }

      resolve();
    });
  });
}

function rweArgsFromGameRoom(game: GameRoom): RweArgs {
  const playersArgs = game.players.map((x, i) => {
    const controller: RweArgsPlayerController =
      x.id === game.localPlayerId
      ? { type: "human" }
      : { type: "remote", host: x.host, port: (6670 + i) };
    const a: RweArgsPlayerInfo = {
      side: x.side,
      color: x.color,
      controller,
    };
    return a;
  });
  const portOffset = game.players.findIndex(x => x.id === game.localPlayerId);
  return {
    map: "Evad River Confluence",
    port: (6670 + portOffset),
    players: playersArgs,
  };
}

export function startGameThunk(): ThunkAction<void, State, void, AppAction>  {
  return (dispatch: Dispatch, getState: () => State) => {
    const state = getState();
    if (!state.currentGame) {
      return;
    }
    dispatch(startGame());
    execRwe(rweArgsFromGameRoom(state.currentGame))
    .then(() => dispatch(gameEnded()), () => dispatch(gameEnded()));
  }
}

export function launchRwe(): ThunkAction<void, State, void, AppAction>  {
  return (dispatch: Dispatch, getState: () => State) => {
    if (getState().isRweRunning) {
      return;
    }

    dispatch(launchRweBegin());
    execRwe()
    .then(() => dispatch(launchRweEnd()), () => dispatch(launchRweEnd()));
  };
}
