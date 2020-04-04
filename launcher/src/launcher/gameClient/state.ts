import { AppAction } from "../actions";
import { findAndMap, assertNever, choose } from "../../common/util";

export type PlayerSide = "ARM" | "CORE";

export type PlayerColor = number;

export interface PlayerInfo {
  id: number;
  name: string;
  side: PlayerSide;
  color: PlayerColor;
  team?: number;
  ready: boolean;
  installedMods: string[];
}

export interface ChatMessage {
  senderName?: string;
  message: string;
}

export interface FilledPlayerSlot {
  state: "filled";
  player: PlayerInfo;
}

export interface EmptyPlayerSlot {
  state: "empty";
}

export interface ClosedPlayerSlot {
  state: "closed";
}

export type PlayerSlot = EmptyPlayerSlot | ClosedPlayerSlot | FilledPlayerSlot;

export interface CurrentGameState {
  localPlayerId?: number;
  adminPlayerId?: number;
  players: PlayerSlot[];
  messages: ChatMessage[];
  mapName?: string;
  activeMods: string[];
}

export type CanStartGameError =
  | { type: "not-admin" }
  | { type: "no-players" }
  | { type: "no-map" }
  | { type: "no-mods" }
  | { type: "unfilled-slots"; unfilledSlots: number[] }
  | { type: "unready-players"; unreadyPlayerIds: number[] }
  | {
      type: "missing-mods";
      playersMissingMods: { playerId: number; mods: string[] }[];
    };

export type CanStartGameResult =
  | { result: "ok" }
  | { result: "err"; errors: CanStartGameError[] };

export function canStartGame(room: CurrentGameState): CanStartGameResult {
  const errors: CanStartGameError[] = [];

  if (room.adminPlayerId !== room.localPlayerId) {
    errors.push({ type: "not-admin" });
  }
  if (room.players.filter(x => x.state === "filled").length < 2) {
    errors.push({ type: "no-players" });
  }

  if (room.activeMods.length === 0) {
    errors.push({ type: "no-mods" });
  }

  if (room.mapName === undefined) {
    errors.push({ type: "no-map" });
  }

  const unfilledSlots = choose(room.players, (x, i) =>
    x.state === "empty" ? i : undefined
  );
  if (unfilledSlots.length > 0) {
    errors.push({ type: "unfilled-slots", unfilledSlots });
  }

  const unreadyPlayerIds = choose(room.players, x => {
    switch (x.state) {
      case "filled": {
        if (!x.player.ready) {
          return x.player.id;
        }
        return undefined;
      }
      case "closed":
        return undefined;
      case "empty":
        return undefined;
      default:
        assertNever(x);
    }
  });

  if (unreadyPlayerIds.length !== 0) {
    errors.push({ type: "unready-players", unreadyPlayerIds });
  }

  const playersMissingMods = choose(room.players, x => {
    switch (x.state) {
      case "filled": {
        const missingMods = room.activeMods.filter(
          m => !x.player.installedMods.includes(m)
        );
        if (missingMods.length !== 0) {
          return { playerId: x.player.id, mods: missingMods };
        }
        return undefined;
      }
      case "closed":
        return undefined;
      case "empty":
        return undefined;
      default:
        assertNever(x);
    }
  });

  if (playersMissingMods.length !== 0) {
    errors.push({ type: "missing-mods", playersMissingMods });
  }

  return errors.length === 0 ? { result: "ok" } : { result: "err", errors };
}

function findPlayer(
  players: PlayerSlot[],
  playerId: number
): PlayerInfo | undefined {
  return findAndMap(players, x =>
    x.state === "filled" && x.player.id === playerId ? x.player : undefined
  );
}

function currentGameReducer(
  room: CurrentGameState,
  action: AppAction
): CurrentGameState {
  switch (action.type) {
    case "RECEIVE_PLAYER_JOINED": {
      const newPlayer: PlayerSlot = {
        state: "filled",
        player: {
          id: action.payload.playerId,
          name: action.payload.name,
          side: "ARM",
          color: 0,
          team: 0,
          ready: false,
          installedMods: action.payload.installedMods,
        },
      };
      const newPlayerIndex = room.players.findIndex(x => x.state === "empty");
      if (newPlayerIndex === -1) {
        throw new Error("Player joined game, but already full!");
      }
      const newPlayers = room.players.map((x, i) =>
        i === newPlayerIndex ? newPlayer : x
      );
      return { ...room, players: newPlayers };
    }
    case "RECEIVE_PLAYER_LEFT": {
      const newPlayers = room.players.map(x => {
        if (x.state === "filled" && x.player.id === action.payload.playerId) {
          const e: EmptyPlayerSlot = { state: "empty" };
          return e;
        }
        return x;
      });
      const newAdminId =
        action.payload.newAdminPlayerId !== undefined
          ? action.payload.newAdminPlayerId
          : action.payload.playerId === room.adminPlayerId
          ? undefined
          : room.adminPlayerId;

      return { ...room, players: newPlayers, adminPlayerId: newAdminId };
    }
    case "RECEIVE_CHAT_MESSAGE": {
      const newMessages = room.messages.slice();
      const sender = findPlayer(room.players, action.payload.playerId);
      const senderName = sender ? sender.name : undefined;
      const newMessage: ChatMessage = {
        senderName: senderName,
        message: action.payload.message,
      };
      newMessages.push(newMessage);
      return { ...room, messages: newMessages };
    }
    case "RECEIVE_PLAYER_CHANGED_SIDE": {
      const newPlayers = room.players.map(x => {
        if (x.state !== "filled" || x.player.id !== action.payload.playerId) {
          return x;
        }
        const p = { ...x.player, side: action.payload.side };
        return { ...x, player: p };
      });
      return { ...room, players: newPlayers };
    }
    case "RECEIVE_PLAYER_CHANGED_TEAM": {
      const newPlayers = room.players.map(x => {
        if (x.state !== "filled" || x.player.id !== action.payload.playerId) {
          return x;
        }
        const p = { ...x.player, team: action.payload.team };
        return { ...x, player: p };
      });
      return { ...room, players: newPlayers };
    }
    case "RECEIVE_PLAYER_CHANGED_COLOR": {
      const newPlayers = room.players.map(x => {
        if (x.state !== "filled" || x.player.id !== action.payload.playerId) {
          return x;
        }
        const p = { ...x.player, color: action.payload.color };
        return { ...x, player: p };
      });
      return { ...room, players: newPlayers };
    }
    case "RECEIVE_PLAYER_READY": {
      const newPlayers = room.players.map(x => {
        if (x.state !== "filled" || x.player.id !== action.payload.playerId) {
          return x;
        }
        const p = { ...x.player, ready: action.payload.value };
        return { ...x, player: p };
      });
      return { ...room, players: newPlayers };
    }
    case "RECEIVE_SLOT_OPENED": {
      const newPlayers = room.players.map((x, i) => {
        if (i !== action.payload.slotId) {
          return x;
        }
        const e: EmptyPlayerSlot = { state: "empty" };
        return e;
      });
      return { ...room, players: newPlayers };
    }
    case "RECEIVE_SLOT_CLOSED": {
      const newPlayers = room.players.map((x, i) => {
        if (i !== action.payload.slotId) {
          return x;
        }
        const e: ClosedPlayerSlot = { state: "closed" };
        return e;
      });
      return { ...room, players: newPlayers };
    }
    case "RECEIVE_ACTIVE_MODS_CHANGED": {
      return { ...room, activeMods: action.payload.mods };
    }
    case "RECEIVE_MAP_CHANGED": {
      return { ...room, mapName: action.data.mapName };
    }
    default: {
      return room;
    }
  }
}

export function currentGameWrapperReducer(
  state: CurrentGameState | undefined,
  action: AppAction
): CurrentGameState | undefined {
  switch (action.type) {
    case "RECEIVE_HANDSHAKE_RESPONSE": {
      const game: CurrentGameState = {
        players: action.payload.players,
        localPlayerId: action.payload.playerId,
        adminPlayerId: action.payload.adminPlayerId,
        mapName: action.payload.mapName,
        messages: [],
        activeMods: action.payload.activeMods,
      };
      return game;
    }
    case "DISCONNECT_GAME": {
      return undefined;
    }
    default: {
      if (!state) {
        return state;
      }
      return currentGameReducer(state, action);
    }
  }
}
