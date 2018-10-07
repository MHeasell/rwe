export interface OverviewScreen {
  screen: "overview";
  dialogOpen: boolean;
}

export interface HostFormScreen {
  screen: "host-form";
}

export interface GameRoomScreen {
  screen: "game-room";
  userMessage: string;
}

export type AppScreen =
  | HostFormScreen
  | OverviewScreen
  | GameRoomScreen;

export type PlayerSide = "ARM" | "CORE";
export type PlayerColor = number;

export interface PlayerInfo {
  id: number;
  name: string;
  host: string;
  side: PlayerSide;
  color: PlayerColor;
  team?: number;
  ready: boolean;
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

export interface GameRoom {
  localPlayerId?: number;
  adminPlayerId?: number;
  players: PlayerSlot[];
  messages: ChatMessage[];
}

export function canStartGame(room: GameRoom) {
  if (room.adminPlayerId !== room.localPlayerId) {
    return false;
  }
  if (room.players.length === 0) {
    return false;
  }

  return room.players.every(x => x.state === "filled" && x.player.ready);
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

export interface State {
  games: GameListEntry[];
  selectedGameId?: number;
  currentScreen: AppScreen;
  isRweRunning: boolean;
  currentGame?: GameRoom;
  masterServerConnectionStatus: MasterServerConnectionStatus;
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
