import { MapDialogState } from "./mapsDialog";
import { WizardState } from "./wizard";
import { RweConfig } from "./rweConfig";

export interface OverviewScreen {
  screen: "overview";
}

export interface HostFormScreen {
  screen: "host-form";
}

export interface GameRoomScreen {
  screen: "game-room";
  room?: GameRoom;
}

export interface MapCacheValue {
  description: string;
  memory: string;
  numberOfPlayers: string;
  minimap?: string;
}

export interface GameRoom {
  mapDialog?: MapDialogState;
  mapCache: { [key: string]: MapCacheValue };
}

export type AppScreen = HostFormScreen | OverviewScreen | GameRoomScreen;

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

export function getRoom(state: State): GameRoom | undefined {
  if (state.currentScreen.screen !== "game-room") {
    return undefined;
  }
  return state.currentScreen.room;
}

export function canStartGame(room: CurrentGameState) {
  if (room.adminPlayerId !== room.localPlayerId) {
    return false;
  }
  if (room.players.length === 0) {
    return false;
  }

  if (room.mapName === undefined) {
    return false;
  }

  return room.players.every(x => {
    switch (x.state) {
      case "filled":
        return (
          x.player.ready &&
          room.activeMods.every(m => x.player.installedMods.includes(m))
        );
      case "closed":
        return true;
      case "empty":
        return false;
    }
  });
}

export interface GameListEntry {
  id: number;
  description: string;
  players: number;
  maxPlayers: number;
}

export function isFull(game: GameListEntry): boolean {
  return game.players === game.maxPlayers;
}

export type MasterServerConnectionStatus = "connected" | "disconnected";

export interface InstalledModInfo {
  name: string;
  path: string;
}

export interface VideoMode {
  width: number;
  height: number;
}

export interface CurrentGameState {
  localPlayerId?: number;
  adminPlayerId?: number;
  players: PlayerSlot[];
  messages: ChatMessage[];
  mapName?: string;
  activeMods: string[];
}

export interface State {
  installedMods?: InstalledModInfo[];
  videoModes?: VideoMode[];
  activeMods: string[];
  games: GameListEntry[];
  selectedGameId?: number;
  currentGame?: CurrentGameState;
  currentScreen: AppScreen;
  isRweRunning: boolean;
  masterServerConnectionStatus: MasterServerConnectionStatus;
  wizard?: WizardState;
  rweConfig?: RweConfig;
}

export function canJoinSelectedGame(state: State): boolean {
  if (state.isRweRunning) {
    return false;
  }

  if (state.selectedGameId === undefined) {
    return false;
  }

  const game = state.games.find(g => g.id === state.selectedGameId);
  if (!game || isFull(game)) {
    return false;
  }

  return true;
}

export function canHostGame(state: State): boolean {
  return true;
}

export function canLaunchRwe(state: State): boolean {
  return !state.isRweRunning;
}
