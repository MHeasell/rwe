import { AppAction, disconnectGame, receiveHandshakeResponse, receivePlayerJoined, receivePlayerLeft, receiveChatMessage, receivePlayerReady, receiveStartGame, gameEnded, LaunchRweAction } from "../actions";
import { StateObservable, combineEpics, ofType } from "redux-observable";
import { State, GameRoom } from "../state";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";

import { GameHostService } from "../ws/game-server";
import { GameClientService } from "../ws/game-client";

import { RweArgs, RweArgsPlayerController, RweArgsPlayerInfo, execRwe } from "../rwe";

export interface EpicDependencies {
  hostService: GameHostService;
  clientService: GameClientService;
}

function looksLikeIPv6Address(value: string) {
  return value.match(/^[0-9a-fA-F:]+$/) && value.match(/:/);
}

const gameClientEventsEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, {clientService, hostService}: EpicDependencies): rx.Observable<AppAction> => {
  return rx.merge<AppAction>(
    clientService.onDisconnect.pipe(rxop.map(disconnectGame)),
    clientService.onHandshakeResponse.pipe(rxop.map(receiveHandshakeResponse)),
    clientService.onPlayerJoined.pipe(rxop.map(receivePlayerJoined)),
    clientService.onPlayerLeft.pipe(rxop.map(receivePlayerLeft)),
    clientService.onPlayerChatMessage.pipe(rxop.map(receiveChatMessage)),
    clientService.onPlayerReady.pipe(rxop.map(receivePlayerReady)),
    clientService.onStartGame.pipe(rxop.map(receiveStartGame)),
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

const launchRweEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, {clientService, hostService}: EpicDependencies): rx.Observable<AppAction> => {
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

const gameRoomEpic = (action$: rx.Observable<AppAction>, state$: StateObservable<State>, {clientService, hostService}: EpicDependencies): rx.Observable<AppAction> => {
  return action$.pipe(
    rxop.flatMap(action => {
      switch (action.type) {
        case "HOST_GAME_FORM_CONFIRM": {
          hostService.createServer(action.gameDescription, action.players, 1337);
          clientService.connectToServer("http://localhost:1337/", action.playerName);
          break;
        }
        case "JOIN_SELECTED_GAME_CONFIRM": {
          const state = state$.value;
          if (state.selectedGameId === undefined) { break; }
          const gameInfo = state.games.find(x => x.id === state.selectedGameId)!;
          // hackily detect IPv6 addressses and enclose them in []
          const address = looksLikeIPv6Address(gameInfo.host) ? `[${gameInfo.host}]` : gameInfo.host;
          const connectionString = `http://${address}:${gameInfo.port}/`;
          console.log(`connecting to ${connectionString}`);
          clientService.connectToServer(connectionString, action.name);
          break;
        }
        case "SEND_CHAT_MESSAGE": {
          clientService.sendChatMessage(action.message);
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
          hostService.destroyServer();
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

export const rootEpic = combineEpics(gameClientEventsEpic, gameRoomEpic, launchRweEpic);
