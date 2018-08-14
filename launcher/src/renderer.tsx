// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

import CssBaseline from "@material-ui/core/CssBaseline";
import * as React from "react";
import * as ReactDOM from "react-dom";
import { Provider } from "react-redux";
import { applyMiddleware, createStore, Store, compose, MiddlewareAPI, Middleware, Dispatch } from "redux";
import thunk from "redux-thunk";
import { receiveRooms, AppAction, startGameThunk } from "./actions";
import App from "./components/App";
import rootReducer, { State } from "./reducers";
import { getRooms } from "./web";

import "./style.css";
import { GameHostService, GameClientService } from "./game-server";

function looksLikeIPv6Address(value: string) {
  return value.match(/^[0-9a-fA-F:]+$/) && value.match(/:/);
}

const gameHoster: Middleware = (store: MiddlewareAPI<Dispatch, State>) => {
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
        clientService.startGame();
        break;
      }
      case "RECEIVE_START_GAME": {
        store.dispatch<any>(startGameThunk());
        break;
      }
    }
  };
};

const composeEnhancers = (window as any).__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ || compose;
const store: Store<any> = createStore(rootReducer, composeEnhancers(applyMiddleware(thunk, gameHoster)));

function doPollLoop() {
  getRooms()
  .then(rooms => {
    store.dispatch(receiveRooms(rooms));
  })
  .then(() => setTimeout(doPollLoop, 5000), () => setTimeout(doPollLoop, 5000));
}

doPollLoop();

ReactDOM.render(
  <React.Fragment>
    <CssBaseline />
    <Provider store={store}>
      <App />
    </Provider>
  </React.Fragment>,
  document.getElementById("app"));
