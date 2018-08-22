type PlayerSide = "ARM" | "CORE";
type PlayerColor = number;

export interface PlayerInfo {
  id: number;
  name: string;
  host: string;
  side: PlayerSide;
  color: PlayerColor;
  team: number;
  ready: boolean;
}

// Emitted by the client upon connection
export const Handshake = "handshake";
export interface HandshakePayload {
  name: string;
}

// Emitted by the server to a client in response to a handshake
export const HandshakeResponse = "handshake-response";
export interface HandshakeResponsePayload {
  playerId: number;
  adminPlayerId: number;
  players: PlayerInfo[];
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
  host: string;
}

// Broadcast by the server to all clients when a player leaves
export const PlayerLeft = "player-left";
export interface PlayerLeftPayload {
  playerId: number;
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
