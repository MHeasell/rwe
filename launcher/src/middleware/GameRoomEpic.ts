import { AppAction, disconnectGame, receiveHandshakeResponse, receivePlayerJoined, receivePlayerLeft, receiveChatMessage, receivePlayerReady, startGameThunk, receiveStartGame } from "../actions";
import { StateObservable, combineEpics } from "redux-observable";
import { State } from "../state";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";

import { GameHostService } from "../ws/game-server";
import { GameClientService } from "../ws/game-client";

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
          const gameInfo = state.games.find(x => x.id == state.selectedGameId)!;
          // hackily detect IPv6 addressses and enclose them in []
          let address = looksLikeIPv6Address(gameInfo.host) ? `[${gameInfo.host}]` : gameInfo.host;
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
          return rx.of(startGameThunk()) as any;
        }
      }
      return rx.empty();
    }),
  );
};

export const rootEpic = combineEpics(gameClientEventsEpic, gameRoomEpic);
