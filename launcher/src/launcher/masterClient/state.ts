export interface GameListEntry {
  id: number;
  description: string;
  players: number;
  maxPlayers: number;
}

export type MasterServerConnectionStatus = "connected" | "disconnected";

export interface MasterClientState {
  games: GameListEntry[];
  masterServerConnectionStatus: MasterServerConnectionStatus;
}

export function isFull(game: GameListEntry): boolean {
  return game.players === game.maxPlayers;
}
