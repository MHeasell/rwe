import { AppAction, disconnectGame, receiveHandshakeResponse, receivePlayerJoined, receivePlayerLeft, receiveChatMessage, receivePlayerReady, receiveStartGame, gameEnded, LaunchRweAction, receiveRooms, receiveGameCreated, receiveGameUpdated, receiveGameDeleted, receiveCreateGameResponse, ReceiveCreateGameResponseAction, masterServerConnect, masterServerDisconnect, receivePlayerChangedSide, receivePlayerChangedColor } from "../actions";
import { StateObservable, combineEpics, ofType } from "redux-observable";
import { State, GameRoom } from "../state";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";

import { GameClientService } from "../ws/game-client";

import { RweArgs, RweArgsPlayerController, RweArgsPlayerInfo, execRwe } from "../rwe";
import { MasterClientService } from "../master/master-client";
import { masterServer } from "../util";

export interface EpicDependencies {
  clientService: GameClientService;
  masterClentService: MasterClientService;
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
    clientService.onPlayerChangedColor.pipe(rxop.map(receivePlayerChangedColor)),
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
  const playersArgs = game.players.map((x, i) => {
    const controller: RweArgsPlayerController =
      x.id === game.localPlayerId
      ? { type: "human" }
      : { type: "remote", host: x.host, port: (6670 + i) };
    const a: RweArgsPlayerInfo = {
      side: x.side,
      color: x.color,
      controller,
    };
    return a;
  });
  const portOffset = game.players.findIndex(x => x.id === game.localPlayerId);
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
        case "CHANGE_COLOR": {
          clientService.changeColor(action.color);
          break;
        }
        case "TOGGLE_READY": {
          const state = state$.value;
          if (!state.currentGame) { break; }
          if (state.currentGame.localPlayerId === undefined) { break; }
          const currentValue = state.currentGame.players.find(x => x.id === state.currentGame!.localPlayerId)!.ready;
          clientService.setReadyState(!currentValue);
          break;
        }
        case "LEAVE_GAME": {
          clientService.disconnect();
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

export const rootEpic = combineEpics(masterClientEventsEpic, gameClientEventsEpic, gameRoomEpic, launchRweEpic);
