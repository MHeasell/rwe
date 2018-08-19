import { MiddlewareAPI, Middleware, Dispatch } from "redux";
import { AppAction, startGameThunk } from "../actions";
import { State } from "../state";

import { GameHostService } from "../ws/game-server";
import { GameClientService } from "../ws/game-client";

function looksLikeIPv6Address(value: string) {
  return value.match(/^[0-9a-fA-F:]+$/) && value.match(/:/);
}

export const gameRoomMiddleware: Middleware = (store: MiddlewareAPI<Dispatch, State>) => {
  const hostService = new GameHostService(store);
  const clientService = new GameClientService(store);
  return (next: (action: AppAction) => void) => (action: AppAction) => {
    next(action);

    switch (action.type) {
      case "HOST_GAME_FORM_CONFIRM": {
        hostService.createServer(action.gameDescription, action.players, 1337);
        clientService.connectToServer("http://localhost:1337/", action.playerName);
        break;
      }
      case "JOIN_SELECTED_GAME_CONFIRM": {
        const state = store.getState();
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
        const state = store.getState();
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
        store.dispatch<any>(startGameThunk());
        break;
      }
    }
  };
};
