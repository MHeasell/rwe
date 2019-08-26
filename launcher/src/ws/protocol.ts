type PlayerSide = "ARM" | "CORE";
type PlayerColor = number;

export interface PlayerInfo {
  id: number;
  name: string;
  side: PlayerSide;
  color: PlayerColor;
  team?: number;
  ready: boolean;
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

// Emitted by the client upon connection
export const Handshake = "handshake";
export interface HandshakePayload {
  gameId: number;
  name: string;
  ipv4Address: string;
  adminKey?: string;
}

// Emitted by the server to a client in response to a handshake
export const HandshakeResponse = "handshake-response";
export interface HandshakeResponsePayload {
  playerId: number;
  adminPlayerId?: number;
  players: PlayerSlot[];
  mapName?: string;
}

// Emitted by the client when the user sends a chat message
export const ChatMessage = "chat-message";
export type ChatMessagePayload = string;

// Broadcast by the server to all clients when a chat message is sent
export const PlayerChatMessage = "player-chat-message";
export interface PlayerChatMessagePayload {
  playerId: number;
  message: string;
}

// Broadcast by the server to all clients when a player joins
export const PlayerJoined = "player-joined";
export interface PlayerJoinedPayload {
  playerId: number;
  name: string;
}

// Broadcast by the server to all clients when a player leaves
export const PlayerLeft = "player-left";
export interface PlayerLeftPayload {
  playerId: number;
  newAdminPlayerId?: number;
}

// Emitted by the client when the player changes ready state
export const Ready = "ready";
export type ReadyPayload = boolean;

// Broadcast by the server to all clients when a player changes ready state
export const PlayerReady = "player-ready";
export interface PlayerReadyPayload {
  playerId: number;
  value: boolean;
}

// Emitted by the client when the player wants to start the game
export const RequestStartGame = "request-start-game";

// Broadcast by the server to all clients to announce the start of the game
export const StartGame = "start-game";
export interface StartGamePayload {
  addresses: [number, string][];
}


export const ChangeSide = "change-side";
export interface ChangeSidePayload {
  side: PlayerSide;
}

export const PlayerChangedSide = "player-changed-side";
export interface PlayerChangedSidePayload {
  playerId: number;
  side: PlayerSide;
}


export const ChangeTeam = "change-team";
export interface ChangeTeamPayload {
  team?: number;
}

export const PlayerChangedTeam = "player-changed-team";
export interface PlayerChangedTeamPayload {
  playerId: number;
  team?: number;
}


export const ChangeColor = "change-color";
export interface ChangeColorPayload {
  color: number;
}

export const PlayerChangedColor = "player-changed-color";
export interface PlayerChangedColorPayload {
  playerId: number;
  color: number;
}

export const OpenSlot = "open-slot";
export interface OpenSlotPayload {
  slotId: number;
}

export const CloseSlot = "close-slot";
export interface CloseSlotPayload {
  slotId: number;
}

export const SlotOpened = "slot-opened";
export interface SlotOpenedPayload {
  slotId: number;
}

export const SlotClosed = "slot-closed";
export interface SlotClosedPayload {
  slotId: number;
}

export const ChangeMap = "change-map";
export interface ChangeMapPayload {
  mapName: string;
}

export const MapChanged = "map-changed";
export interface MapChangedPayload {
  mapName: string;
}
