export interface GetGamesReponseEntry {
  description: string;
  players: number;
  max_players: number;
}

export interface GetGamesResponseItem {
  id: number;
  game: GetGamesReponseEntry;
}

export const GetGamesResponse = "get-games-response";
export interface GetGamesResponsePayload {
  games: GetGamesResponseItem[];
}

export const CreateGameRequest = "create-game";
export interface CreateGameRequestPayload {
  description: string;
  max_players: number;
}
export const CreateGameResponse = "create-game-response";
export interface CreateGameResponsePayload {
  game_id: number;
  admin_key: string;
}

export const GameCreatedEvent = "game-created";
export interface GameCreatedEventPayload {
  game_id: number;
  game: GetGamesReponseEntry;
}

export const GameUpdatedEvent = "game-updated";
export interface GameUpdatedEventPayload {
  game_id: number;
  game: GetGamesReponseEntry;
}

export const GameDeletedEvent = "game-deleted";
export interface GameDeletedEventPayload {
  game_id: number;
}
