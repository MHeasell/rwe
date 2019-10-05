import { AppAction } from "./actions";
import { GetGamesResponseItem } from "../master-server/protocol";
import {
  AppScreen,
  ChatMessage,
  ClosedPlayerSlot,
  EmptyPlayerSlot,
  GameListEntry,
  GameRoom,
  GameRoomScreen,
  HostFormScreen,
  OverviewScreen,
  PlayerInfo,
  PlayerSlot,
  State,
  MapCacheValue,
} from "./state";
import { findAndMap } from "../common/util";
import { mapDialogReducer } from "./mapsDialog";
import { wizardReducer } from "./wizard";

const initialState: State = {
  games: [],
  currentScreen: {
    screen: "overview",
  },
  isRweRunning: false,
  masterServerConnectionStatus: "disconnected",
  activeMods: [],
};

function roomResponseEntryToGamesListEntry(
  room: GetGamesResponseItem
): GameListEntry {
  return {
    id: room.id,
    description: room.game.description,
    players: room.game.players,
    maxPlayers: room.game.max_players,
  };
}

function findPlayer(
  players: PlayerSlot[],
  playerId: number
): PlayerInfo | undefined {
  return findAndMap(players, x =>
    x.state === "filled" && x.player.id === playerId ? x.player : undefined
  );
}

function gameRoomScreenReducer(
  screen: GameRoomScreen,
  action: AppAction
): AppScreen {
  switch (action.type) {
    case "RECEIVE_HANDSHAKE_RESPONSE": {
      const room: GameRoom = {
        players: action.payload.players,
        localPlayerId: action.payload.playerId,
        adminPlayerId: action.payload.adminPlayerId,
        mapName: action.payload.mapName,
        messages: [],
        activeMods: action.payload.activeMods,
        mapCache: {},
        modsDialogOpen: false,
      };
      return { ...screen, room };
    }
    case "START_GAME": {
      return { screen: "overview" };
    }
    case "DISCONNECT_GAME": {
      return { screen: "overview" };
    }
    default: {
      if (!screen.room) {
        return screen;
      }
      const room = gameRoomReducer(screen.room, action);
      if (room === screen.room) {
        return screen;
      }
      return { ...screen, room };
    }
  }
}

function overviewScreenReducer(
  screen: OverviewScreen,
  action: AppAction
): AppScreen {
  switch (action.type) {
    case "JOIN_SELECTED_GAME_CONFIRM": {
      return { screen: "game-room" };
    }
    case "HOST_GAME": {
      return { screen: "host-form" };
    }
    default:
      return screen;
  }
}

function hostFormReducer(screen: HostFormScreen, action: AppAction): AppScreen {
  switch (action.type) {
    case "HOST_GAME_FORM_CONFIRM": {
      return { screen: "game-room" };
    }
    case "HOST_GAME_FORM_CANCEL": {
      return { screen: "overview" };
    }
    default:
      return screen;
  }
}

function currentScreenReducer(screen: AppScreen, action: AppAction): AppScreen {
  switch (screen.screen) {
    case "game-room":
      return gameRoomScreenReducer(screen, action);
    case "overview":
      return overviewScreenReducer(screen, action);
    case "host-form":
      return hostFormReducer(screen, action);
  }
}

function mapDialogWrapperReducer(room: GameRoom, action: AppAction): GameRoom {
  switch (action.type) {
    case "OPEN_SELECT_MAP_DIALOG": {
      return { ...room, mapDialog: { selectedMap: room.mapName } };
    }
    case "CLOSE_SELECT_MAP_DIALOG": {
      return { ...room, mapDialog: undefined };
    }
    default: {
      if (room.mapDialog) {
        const mapDialog = mapDialogReducer(room.mapDialog, action);
        if (mapDialog !== room.mapDialog) {
          return { ...room, mapDialog };
        }
      }
      return room;
    }
  }
}

function modDialogWrapperReducer(room: GameRoom, action: AppAction): GameRoom {
  switch (action.type) {
    case "OPEN_SELECT_MODS_DIALOG": {
      return { ...room, modsDialogOpen: true };
    }
    case "CLOSE_SELECT_MODS_DIALOG": {
      return { ...room, modsDialogOpen: false };
    }
    case "REQUEST_SET_ACTIVE_MODS": {
      return { ...room, modsDialogOpen: false };
    }
    default: {
      return room;
    }
  }
}

function wizardWrapperReducer(state: State, action: AppAction): State {
  switch (action.type) {
    case "wizard/OPEN": {
      return { ...state, wizard: { step: "welcome" } };
    }
    case "wizard/CLOSE": {
      if (!state.wizard || state.wizard.step === "working") {
        return state;
      }
      return { ...state, wizard: undefined };
    }
    default:
      if (state.wizard) {
        const wizard = wizardReducer(state.wizard, action);
        if (wizard !== state.wizard) {
          return { ...state, wizard };
        }
      }
      return state;
  }
}

function gameRoomReducer(room: GameRoom, action: AppAction): GameRoom {
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
      return { ...room, activeMods: action.payload.mods, mapCache: {} };
    }
    case "RECEIVE_MAP_CHANGED": {
      return { ...room, mapName: action.data.mapName };
    }
    case "RECEIVE_COMBINED_MAP_INFO": {
      const entry: MapCacheValue = {
        description: action.info.description,
        memory: action.info.memory,
        numberOfPlayers: action.info.numberOfPlayers,
        minimap: action.minimapPath,
      };
      return { ...room, mapCache: { ...room.mapCache, [action.name]: entry } };
    }

    default: {
      return modDialogWrapperReducer(
        mapDialogWrapperReducer(room, action),
        action
      );
    }
  }
}

function games(state: State = initialState, action: AppAction): State {
  switch (action.type) {
    case "SELECT_GAME":
      return { ...state, selectedGameId: action.gameId };
    case "LAUNCH_RWE":
      return { ...state, isRweRunning: true };
    case "LAUNCH_RWE_END":
      return { ...state, isRweRunning: false };
    case "RECEIVE_ROOMS": {
      const gamesList = action.rooms.games.map(
        roomResponseEntryToGamesListEntry
      );
      const selectedId =
        state.selectedGameId &&
        gamesList.find(x => x.id === state.selectedGameId)
          ? state.selectedGameId
          : undefined;
      return { ...state, games: gamesList, selectedGameId: selectedId };
    }
    case "RECEIVE_GAME_CREATED": {
      const game = roomResponseEntryToGamesListEntry({
        id: action.payload.game_id,
        game: action.payload.game,
      });
      const games = state.games.slice();
      games.push(game);
      return { ...state, games };
    }
    case "RECEIVE_GAME_UPDATED": {
      const game = roomResponseEntryToGamesListEntry({
        id: action.payload.game_id,
        game: action.payload.game,
      });
      const games = state.games.map(x => (x.id === game.id ? game : x));
      return { ...state, games };
    }
    case "RECEIVE_GAME_DELETED": {
      const games = state.games.filter(x => x.id !== action.payload.game_id);
      const selectedId =
        state.selectedGameId === action.payload.game_id
          ? undefined
          : state.selectedGameId;
      return { ...state, games, selectedGameId: selectedId };
    }
    case "GAME_ENDED":
      return { ...state, isRweRunning: false };
    case "MASTER_SERVER_CONNECT":
      return { ...state, masterServerConnectionStatus: "connected" };
    case "MASTER_SERVER_DISCONNECT":
      return { ...state, masterServerConnectionStatus: "disconnected" };
    case "RECEIVE_INSTALLED_MODS":
      return { ...state, installedMods: action.mods };
    case "CHANGE_SINGLE_PLAYER_MODS":
      return currentScreenWrapperReducer(
        action.newMods ? { ...state, activeMods: action.newMods } : state,
        action
      );
    default: {
      return wizardWrapperReducer(
        currentScreenWrapperReducer(state, action),
        action
      );
    }
  }
}

function currentScreenWrapperReducer(state: State, action: AppAction): State {
  const screen = currentScreenReducer(state.currentScreen, action);
  if (screen === state.currentScreen) {
    return state;
  }
  return { ...state, currentScreen: screen };
}

export default games;
