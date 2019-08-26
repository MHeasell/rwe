import * as path from "path";
import { combineEpics, ofType, StateObservable } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction, closeSelectMapDialog, disconnectGame, gameEnded, LaunchRweAction, masterServerConnect, masterServerDisconnect, receiveChatMessage, receiveCreateGameResponse, ReceiveCreateGameResponseAction, receiveGameCreated, receiveGameDeleted, receiveGameUpdated, receiveHandshakeResponse, receiveMapChanged, receiveMapInfo, receiveMapList, receiveMinimap, receivePlayerChangedColor, receivePlayerChangedSide, receivePlayerChangedTeam, receivePlayerJoined, receivePlayerLeft, receivePlayerReady, receiveRooms, receiveSlotClosed, receiveSlotOpened, receiveStartGame, LeaveGameAction } from "../actions";
import { FilledPlayerSlot, GameRoom, getRoom, State } from "../state";
import * as protocol from "../ws/protocol";

import { GameClientService } from "../ws/game-client";

import { RweBridge } from "../bridge";
import { getIpv4Address } from "../ip-lookup";
import { MasterClientService } from "../master/master-client";
import { execRwe, RweArgs, RweArgsEmptyPlayerSlot, RweArgsFilledPlayerSlot, RweArgsPlayerController, RweArgsPlayerInfo, RweArgsPlayerSlot } from "../rwe";
import { assertNever, masterServer } from "../util";

export interface EpicDependencies {
  clientService: GameClientService;
  masterClentService: MasterClientService;
  bridgeService: RweBridge;
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
    clientService.onMapChanged.pipe(rxop.map(receiveMapChanged)),
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

function rweArgsFromGameRoom(game: GameRoom, startInfo: protocol.StartGamePayload): RweArgs {
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
          : { type: "remote", host: startInfo.addresses.find(([id, _]) => id === x.player.id)![1], port: (6670 + i) };
        const a: RweArgsPlayerInfo = {
          name: x.player.name,
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
  if (game.mapName === undefined) {
    throw new Error("map is not set");
  }
  return {
    map: game.mapName,
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
  if (process.platform === "win32") {
    const appData = process.env["APPDATA"];
    if (appData === undefined) {
      throw new Error("Failed to find AppData path");
    }
    return path.join(appData, "RWE", "Data");
  }
  else {
    const home = process.env["HOME"];
    if (home === undefined) {
      throw new Error("Failed to find home directory");
    }
    return path.join(home, ".rwe", "Data");
  }
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
            getIpv4Address().then(clientAddress => {
              clientService.connectToServer(`${masterServer()}/rooms`, x.payload.game_id, action.playerName, clientAddress, x.payload.admin_key);
            });
          });
          break;
        }
        case "JOIN_SELECTED_GAME_CONFIRM": {
          const state = state$.value;
          if (state.selectedGameId === undefined) { break; }
          const selectedGameId = state.selectedGameId;
          const connectionString = `${masterServer()}/rooms`;
          console.log(`connecting to ${connectionString}`);
          deps.bridgeService.addDataPath(getDefaultDataPath());
          rx.from(getIpv4Address()).pipe(
            rxop.takeUntil(action$.pipe(ofType<AppAction, LeaveGameAction>("LEAVE_GAME"))))
            .subscribe(clientAddress => {
              clientService.connectToServer(connectionString, selectedGameId, action.name, clientAddress);
            });
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
          const room = getRoom(state);
          if (!room) { break; }
          if (room.localPlayerId === undefined) { break; }
          const localPlayerSlot = room.players.find(x => x.state === "filled" && x.player.id === room.localPlayerId)! as FilledPlayerSlot;
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
          const room = getRoom(state);
          if (!room) { break; }
          return rx.from(execRwe(rweArgsFromGameRoom(room, action.payload)))
          .pipe(
            rxop.mapTo(undefined),
            rxop.catchError(e => rx.of(undefined)),
            rxop.mapTo(gameEnded()),
          );
        }
        case "CHANGE_MAP": {
          const state = state$.value;
          const room = getRoom(state);
          if (!room) { break; }
          if (!room.mapDialog) { break; }
          if (!room.mapDialog.selectedMap) { break; }
          clientService.changeMap(room.mapDialog.selectedMap.name);
          return rx.of<AppAction>(closeSelectMapDialog());
        }
      }
      return rx.empty();
    }),
  );
};

const rweBridgeEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, deps: EpicDependencies): rx.Observable<AppAction> => {
  const selectedMap$ = state$.pipe(
    rxop.map(state => {
      const room = getRoom(state);
      if (!room) { return undefined; }
      if (!room.mapDialog) { return undefined; }
      if (!room.mapDialog.selectedMap) { return undefined; }
      return room.mapDialog.selectedMap.name;
    }),
    rxop.distinctUntilChanged(),
  );

  const mapInfoStream = selectedMap$.pipe(
    rxop.switchMap((mapName): rx.Observable<AppAction> => {
      if (mapName === undefined) { return rx.empty(); }
      return rx.from(deps.bridgeService.getMapInfo(mapName))
      .pipe(rxop.map(receiveMapInfo));
  }));

  const minimapStream = selectedMap$.pipe(
    rxop.switchMap((mapName): rx.Observable<AppAction> => {
      if (mapName === undefined) { return rx.empty(); }
      return rx.from(deps.bridgeService.getMinimap(mapName))
      .pipe(rxop.map(x => receiveMinimap(x.path)));
  }));

  const actionPipe = action$.pipe(
    rxop.flatMap((action): rx.Observable<AppAction> => {
      switch (action.type) {
        case "OPEN_SELECT_MAP_DIALOG": {
          return rx.from(deps.bridgeService.getMapList())
          .pipe(rxop.map(x => receiveMapList(x.maps)));
        }
      }
      return rx.empty();
    }),
  );

  return rx.merge(mapInfoStream, minimapStream, actionPipe);
};

export const rootEpic = combineEpics(masterClientEventsEpic, gameClientEventsEpic, gameRoomEpic, launchRweEpic, rweBridgeEpic);
