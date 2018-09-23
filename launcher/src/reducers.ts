import { AppAction } from "./actions";
import { State, GameListEntry, GameRoom, PlayerInfo, ChatMessage } from "./state";
import { GetGamesResponseItem } from "./master/protocol";

const initialState: State = {
  games: [],
  currentScreen: { screen: "overview", dialogOpen: false },
  isRweRunning: false,
  masterServerConnectionStatus: "disconnected",
};

function roomResponseEntryToGamesListEntry(room: GetGamesResponseItem): GameListEntry {
  return {
    id: room.id,
    description: room.game.description,
    players: room.game.players,
    maxPlayers: room.game.max_players,
  };
}

function games(state: State = initialState, action: AppAction): State {
  switch (action.type) {
    case "SELECT_GAME":
      return { ...state, selectedGameId: action.gameId };
    case "JOIN_SELECTED_GAME": {
      if (state.currentScreen.screen !== "overview") { return state; }
      const newScreen = { ...state.currentScreen, dialogOpen: true };
      return { ...state, currentScreen: newScreen };
    }
    case "JOIN_SELECTED_GAME_CONFIRM": {
      const game: GameRoom = { players: [], messages: [] };
      return { ...state, currentScreen: { screen: "game-room", userMessage: "" }, currentGame: game };
    }
    case "JOIN_SELECTED_GAME_CANCEL": {
      if (state.currentScreen.screen !== "overview") { return state; }
      const newScreen = { ...state.currentScreen, dialogOpen: false };
      return { ...state, currentScreen: newScreen };
    }
    case "HOST_GAME":
      return { ...state, currentScreen: { screen: "host-form" } };
    case "HOST_GAME_FORM_CANCEL":
      return { ...state, currentScreen: { screen: "overview", dialogOpen: false } };
    case "HOST_GAME_FORM_CONFIRM": {
      const game: GameRoom = { players: [], messages: [] };
      return { ...state, currentScreen: { screen: "game-room", userMessage: "" }, currentGame: game };
    }
    case "LAUNCH_RWE":
      return { ...state, isRweRunning: true };
    case "LAUNCH_RWE_END":
      return { ...state, isRweRunning: false };
    case "RECEIVE_ROOMS": {
      const gamesList = action.rooms.games.map(roomResponseEntryToGamesListEntry);
      const selectedId =
        (state.selectedGameId && gamesList.find(x => x.id === state.selectedGameId))
        ? state.selectedGameId
        : undefined;
      return { ...state, games: gamesList, selectedGameId: selectedId };
    }
    case "RECEIVE_GAME_CREATED": {
      const game = roomResponseEntryToGamesListEntry({ id: action.payload.game_id, game: action.payload.game });
      const games = state.games.slice();
      games.push(game);
      return { ...state, games };
    }
    case "RECEIVE_GAME_UPDATED": {
      const game = roomResponseEntryToGamesListEntry({ id: action.payload.game_id, game: action.payload.game });
      const games = state.games.map(x => x.id === game.id ? game : x);
      return { ...state, games };
    }
    case "RECEIVE_GAME_DELETED": {
      const games = state.games.filter(x => x.id !== action.payload.game_id);
      const selectedId = state.selectedGameId === action.payload.game_id ? undefined : state.selectedGameId;
      return { ...state, games, selectedGameId: selectedId };
    }
    case "RECEIVE_HANDSHAKE_RESPONSE": {
      if (!state.currentGame) { return state; }
      const newRoom = { ...state.currentGame, players: action.payload.players, localPlayerId: action.payload.playerId, adminPlayerId: action.payload.adminPlayerId };
      return { ...state, currentGame: newRoom };
    }
    case "RECEIVE_PLAYER_JOINED": {
      if (!state.currentGame) { return state; }
      const newPlayers = state.currentGame.players.slice();
      const newPlayer: PlayerInfo = {
        id: action.payload.playerId,
        name: action.payload.name,
        host: action.payload.host,
        side: "ARM",
        color: 0,
        team: 0,
        ready: false,
      };
      newPlayers.push(newPlayer);
      const newRoom: GameRoom = { ...state.currentGame, players: newPlayers };
      return { ...state, currentGame: newRoom };
    }
    case "RECEIVE_PLAYER_LEFT": {
      if (!state.currentGame) { return state; }
      const newPlayers = state.currentGame.players.filter(x => x.id !== action.payload.playerId);
      const newAdminId =
        action.payload.newAdminPlayerId !== undefined ? action.payload.newAdminPlayerId
        : action.payload.playerId === state.currentGame.adminPlayerId ? undefined
        : state.currentGame.adminPlayerId;

      const newRoom: GameRoom = { ...state.currentGame, players: newPlayers, adminPlayerId: newAdminId };
      return { ...state, currentGame: newRoom };
    }
    case "RECEIVE_CHAT_MESSAGE": {
      if (!state.currentGame) { return state; }
      const newMessages = state.currentGame.messages.slice();
      const sender = state.currentGame.players.find(x => x.id === action.payload.playerId);
      const senderName = sender ? sender.name : undefined;
      const newMessage: ChatMessage = {
        senderName: senderName,
        message: action.payload.message,
      };
      newMessages.push(newMessage);
      const room: GameRoom = { ...state.currentGame, messages: newMessages };
      return { ...state, currentGame: room };
    }
    case "RECEIVE_PLAYER_CHANGED_SIDE": {
      if (!state.currentGame) { return state; }
      const newPlayers = state.currentGame.players.map(x => {
        if (x.id !== action.payload.playerId) { return x; }
        return { ...x, side: action.payload.side };
      });
      const room: GameRoom = { ...state.currentGame, players: newPlayers };
      return { ...state, currentGame: room };
    }
    case "RECEIVE_PLAYER_READY": {
      if (!state.currentGame) { return state; }
      const newPlayers = state.currentGame.players.map(x => {
        if (x.id !== action.payload.playerId) {
          return x;
        }
        return { ...x, ready: action.payload.value };
      });
      const room: GameRoom = { ...state.currentGame, players: newPlayers };
      return { ...state, currentGame: room };
    }
    case "DISCONNECT_GAME":
      if (!state.currentGame) { return state; }
      return { ...state, currentScreen: { screen: "overview", dialogOpen: false }, currentGame: undefined };
    case "START_GAME":
      if (!state.currentGame) { return state; }
      return { ...state, currentScreen: { screen: "overview", dialogOpen: false }, currentGame: undefined, isRweRunning: true };
    case "GAME_ENDED":
      return { ...state, isRweRunning: false };
    case "MASTER_SERVER_CONNECT":
      return { ...state, masterServerConnectionStatus: "connected" };
    case "MASTER_SERVER_DISCONNECT":
      return { ...state, masterServerConnectionStatus: "disconnected" };
    default:
      return state;
  }
}

export default games;
