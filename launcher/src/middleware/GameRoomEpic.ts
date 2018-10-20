import { AppAction, disconnectGame, receiveHandshakeResponse, receivePlayerJoined, receivePlayerLeft, receiveChatMessage, receivePlayerReady, receiveStartGame, gameEnded, LaunchRweAction, receiveRooms, receiveGameCreated, receiveGameUpdated, receiveGameDeleted, receiveCreateGameResponse, ReceiveCreateGameResponseAction, masterServerConnect, masterServerDisconnect, receivePlayerChangedSide, receivePlayerChangedColor, receivePlayerChangedTeam, receiveSlotOpened, receiveSlotClosed, receiveMapList } from "../actions";
import { StateObservable, combineEpics, ofType } from "redux-observable";
import { State, GameRoom, FilledPlayerSlot } from "../state";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import * as path from "path";

import { GameClientService } from "../ws/game-client";

import { RweArgs, RweArgsPlayerController, RweArgsPlayerInfo, execRwe, RweArgsEmptyPlayerSlot, RweArgsPlayerSlot, RweArgsFilledPlayerSlot } from "../rwe";
import { MasterClientService } from "../master/master-client";
import { masterServer, assertNever } from "../util";
import { RweBridge } from "../bridge";

export interface EpicDependencies {
  clientService: GameClientService;
  masterClentService: MasterClientService;
  bridgeService: RweBridge;
}

function looksLikeIPv6Address(value: string) {
  return value.match(/^[0-9a-fA-F:]+$/) && value.match(/:/);
}

const gameClientEventsEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, {clientService}: EpicDependencies): rx.Observable<AppAction> => {
  return rx.merge<AppAction>(
    clientService.onDisconnect.pipe(rxop.map(disconnectGame)),
    clientService.onHandshakeResponse.pipe(rxop.map(receiveHandshakeResponse)),
    clientService.onPlayerJoined.pipe(rxop.map(receivePlayerJoined)),
    clientService.onPlayerLeft.pipe(rxop.map(receivePlayerLeft)),
    clientService.onPlayerChatMessage.pipe(rxop.map(receiveChatMessage)),
    clientService.onPlayerChangedSide.pipe(rxop.map(receivePlayerChangedSide)),
    clientService.onPlayerChangedTeam.pipe(rxop.map(receivePlayerChangedTeam)),
    clientService.onPlayerChangedColor.pipe(rxop.map(receivePlayerChangedColor)),
    clientService.onSlotOpened.pipe(rxop.map(receiveSlotOpened)),
    clientService.onSlotClosed.pipe(rxop.map(receiveSlotClosed)),
    clientService.onPlayerReady.pipe(rxop.map(receivePlayerReady)),
    clientService.onStartGame.pipe(rxop.map(receiveStartGame)),
  );
};

const masterClientEventsEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, deps: EpicDependencies): rx.Observable<AppAction> => {
  const s = deps.masterClentService;
  return rx.merge<AppAction>(
    s.onConnect.pipe(rxop.map(masterServerConnect)),
    s.onDisconnect.pipe(rxop.map(masterServerDisconnect)),
    s.onGetGamesResponse.pipe(rxop.map(receiveRooms)),
    s.onCreateGameResponse.pipe(rxop.map(receiveCreateGameResponse)),
    s.onGameCreated.pipe(rxop.map(receiveGameCreated)),
    s.onGameUpdated.pipe(rxop.map(receiveGameUpdated)),
    s.onGameDeleted.pipe(rxop.map(receiveGameDeleted)),
  );
};

function rweArgsFromGameRoom(game: GameRoom): RweArgs {
  const playersArgs: RweArgsPlayerSlot[] = game.players.map((x, i) => {
    switch (x.state) {
      case "empty":
      case "closed": {
        const s: RweArgsEmptyPlayerSlot = { state: "empty" };
        return s;
      }
      case "filled": {
        const controller: RweArgsPlayerController =
          x.player.id === game.localPlayerId
          ? { type: "human" }
          : { type: "remote", host: x.player.host, port: (6670 + i) };
        const a: RweArgsPlayerInfo = {
          side: x.player.side,
          color: x.player.color,
          controller,
        };
        const s: RweArgsFilledPlayerSlot = { ...a, state: "filled" };
        return s;
      }
      default: return assertNever(x);
    }
  });
  const portOffset = game.players.findIndex(x => x.state === "filled" && x.player.id === game.localPlayerId);
  return {
    map: "Evad River Confluence",
    port: (6670 + portOffset),
    players: playersArgs,
  };
}

const launchRweEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, _: EpicDependencies): rx.Observable<AppAction> => {
  return action$.pipe(
    ofType<AppAction, LaunchRweAction>("LAUNCH_RWE"),
    rxop.flatMap(() => {
      return rx.from(execRwe())
      .pipe(
        rxop.mapTo(undefined),
        rxop.catchError(e => rx.of(undefined)),
        rxop.mapTo(gameEnded()),
      );
    }),
  );
};

function getDefaultDataPath() {
  const appData = process.env["APPDATA"];
  if (appData === undefined) {
    throw new Error("Failed to find AppData path");
  }
  return path.join(appData, "RWE", "Data");
}

const gameRoomEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, deps: EpicDependencies): rx.Observable<AppAction> => {
  const clientService = deps.clientService;
  const masterClientService = deps.masterClentService;

  return action$.pipe(
    rxop.flatMap(action => {
      switch (action.type) {
        case "HOST_GAME_FORM_CONFIRM": {
          masterClientService.requestCreateGame(action.gameDescription, action.players);
          action$.pipe(
            ofType<AppAction, ReceiveCreateGameResponseAction>("RECEIVE_CREATE_GAME_RESPONSE"),
            rxop.first(),
          ).subscribe(x => {
            deps.bridgeService.addDataPath(getDefaultDataPath());
            clientService.connectToServer(`${masterServer()}/rooms`, x.payload.game_id, action.playerName, x.payload.admin_key);
          });
          break;
        }
        case "JOIN_SELECTED_GAME_CONFIRM": {
          const state = state$.value;
          if (state.selectedGameId === undefined) { break; }
          const gameInfo = state.games.find(x => x.id === state.selectedGameId)!;
          const connectionString = `${masterServer()}/rooms`;
          console.log(`connecting to ${connectionString}`);
          deps.bridgeService.addDataPath(getDefaultDataPath());
          clientService.connectToServer(connectionString, state.selectedGameId, action.name);
          break;
        }
        case "SEND_CHAT_MESSAGE": {
          clientService.sendChatMessage(action.message);
          break;
        }
        case "CHANGE_SIDE": {
          clientService.changeSide(action.side);
          break;
        }
        case "CHANGE_TEAM": {
          clientService.changeTeam(action.team);
          break;
        }
        case "CHANGE_COLOR": {
          clientService.changeColor(action.color);
          break;
        }
        case "OPEN_SLOT": {
          clientService.openSlot(action.slotId);
          break;
        }
        case "CLOSE_SLOT": {
          clientService.closeSlot(action.slotId);
          break;
        }
        case "TOGGLE_READY": {
          const state = state$.value;
          if (!state.currentGame) { break; }
          if (state.currentGame.localPlayerId === undefined) { break; }
          const localPlayerSlot = state.currentGame.players.find(x => x.state === "filled" && x.player.id === state.currentGame!.localPlayerId)! as FilledPlayerSlot;
          const currentValue = localPlayerSlot.player.ready;
          clientService.setReadyState(!currentValue);
          break;
        }
        case "LEAVE_GAME": {
          clientService.disconnect();
          break;
        }
        case "DISCONNECT_GAME": {
          deps.bridgeService.clearDataPaths();
          break;
        }
        case "START_GAME": {
          deps.bridgeService.clearDataPaths();
          break;
        }
        case "SEND_START_GAME": {
          clientService.requestStartGame();
          break;
        }
        case "RECEIVE_START_GAME": {
          const state = state$.value;
          if (!state.currentGame) { break; }
          return rx.from(execRwe(rweArgsFromGameRoom(state.currentGame)))
          .pipe(
            rxop.mapTo(undefined),
            rxop.catchError(e => rx.of(undefined)),
            rxop.mapTo(gameEnded()),
          );
        }
      }
      return rx.empty();
    }),
  );
};

const rweBridgeEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, deps: EpicDependencies): rx.Observable<AppAction> => {
  return action$.pipe(
    rxop.flatMap(action => {
      switch (action.type) {
        case "OPEN_SELECT_MAP_DIALOG": {
          return rx.from(deps.bridgeService.getMapList())
          .pipe(rxop.map(x => receiveMapList(x.maps)));
        }
        default:
          return rx.empty();
      }
    }),
  );
};

export const rootEpic = combineEpics(masterClientEventsEpic, gameClientEventsEpic, gameRoomEpic, launchRweEpic, rweBridgeEpic);
