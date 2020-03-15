import { AppAction } from "./actions";
import { GetGamesResponseItem } from "../master-server/protocol";
import {
  AppScreen,
  GameListEntry,
  GameRoom,
  GameRoomScreen,
  HostFormScreen,
  OverviewScreen,
  State,
  MapCacheValue,
} from "./state";
import { mapDialogReducer } from "./mapsDialog";
import { wizardReducer } from "./wizard";
import { Reducer, Action } from "redux";
import { currentGameWrapperReducer } from "./gameClient/state";

function composeReducers<S, A extends Action>(
  a: (state: S, action: A) => S,
  b: (state: S, action: A) => S
): (state: S, action: A) => S {
  return (state: S, action: A) => a(b(state, action), action);
}

function reduceReducers<S, A extends Action>(
  ...rs: ((state: S, action: A) => S)[]
): (state: S, action: A) => S {
  return rs.reduce(composeReducers);
}

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

function gameRoomScreenReducer(
  screen: GameRoomScreen,
  mapName: string | undefined,
  action: AppAction
): AppScreen {
  switch (action.type) {
    case "RECEIVE_HANDSHAKE_RESPONSE": {
      const room: GameRoom = {
        mapCache: {},
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
      const room = gameRoomReducer(screen.room, mapName, action);
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

function currentScreenReducer(
  screen: AppScreen,
  mapName: string | undefined,
  action: AppAction
): AppScreen {
  switch (screen.screen) {
    case "game-room":
      return gameRoomScreenReducer(screen, mapName, action);
    case "overview":
      return overviewScreenReducer(screen, action);
    case "host-form":
      return hostFormReducer(screen, action);
  }
}

function mapDialogWrapperReducer(
  room: GameRoom,
  mapName: string | undefined,
  action: AppAction
): GameRoom {
  switch (action.type) {
    case "OPEN_SELECT_MAP_DIALOG": {
      return { ...room, mapDialog: { selectedMap: mapName } };
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

function wizardWrapperReducer(
  state: State = initialState,
  action: AppAction
): State {
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

function gameRoomReducer(
  room: GameRoom,
  mapName: string | undefined,
  action: AppAction
): GameRoom {
  switch (action.type) {
    case "RECEIVE_COMBINED_MAP_INFO": {
      const entry: MapCacheValue = {
        description: action.info.description,
        memory: action.info.memory,
        numberOfPlayers: action.info.numberOfPlayers,
        minimap: action.minimapPath,
      };
      return { ...room, mapCache: { ...room.mapCache, [action.name]: entry } };
    }
    case "RECEIVE_ACTIVE_MODS_CHANGED": {
      return { ...room, mapCache: {} };
    }
    default: {
      return mapDialogWrapperReducer(room, mapName, action);
    }
  }
}

function globalActionsReducer(
  state: State = initialState,
  action: AppAction
): State {
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
    case "RECEIVE_VIDEO_MODES":
      return { ...state, videoModes: action.modes };
    case "CHANGE_SINGLE_PLAYER_MODS":
      return currentScreenWrapperReducer(
        action.newMods ? { ...state, activeMods: action.newMods } : state,
        action
      );
    case "RECEIVE_RWE_CONFIG":
      return { ...state, rweConfig: action.settings };
    case "SUBMIT_SETTINGS_DIALOG":
      return { ...state, rweConfig: action.settings };
    default: {
      return state;
    }
  }
}

function currentScreenWrapperReducer(
  state: State = initialState,
  action: AppAction
): State {
  const screen = currentScreenReducer(
    state.currentScreen,
    state.currentGame?.mapName,
    action
  );
  if (screen === state.currentScreen) {
    return state;
  }
  return { ...state, currentScreen: screen };
}

const reduceOnField = <S, A extends Action, K extends keyof S>(
  field: K,
  reducer: Reducer<S[K], A>
) => (state: S, action: A): S => {
  const newFieldValue = reducer(state[field], action);
  if (newFieldValue === state[field]) {
    return state;
  }
  return { ...state, [field]: newFieldValue };
};

const rootReducer = reduceReducers(
  globalActionsReducer,
  wizardWrapperReducer,
  currentScreenWrapperReducer,
  reduceOnField("currentGame", currentGameWrapperReducer)
);

const wrappedRootReducer = (s: State | undefined, a: AppAction) =>
  s === undefined ? initialState : rootReducer(s, a);

export default wrappedRootReducer;
