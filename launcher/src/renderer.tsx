// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

import CssBaseline from "@material-ui/core/CssBaseline";
import * as React from "react";
import * as ReactDOM from "react-dom";
import { Provider } from "react-redux";
import { applyMiddleware, createStore, Store, compose } from "redux";
import { receiveRooms, AppAction } from "./actions";
import App from "./components/App";
import rootReducer from "./reducers";
import { getRooms } from "./web";
import { createEpicMiddleware } from "redux-observable";
import { State } from "./state";
import { rootEpic, EpicDependencies } from "./middleware/GameRoomEpic";
import { GameClientService } from "./ws/game-client";
import { GameHostService } from "./ws/game-server";

import "./style.css";

const epicMiddleware = createEpicMiddleware<AppAction, AppAction, State, EpicDependencies>({
  dependencies: { clientService: new GameClientService(), hostService: new GameHostService() },
});

const composeEnhancers = (window as any).__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ || compose;
const store: Store<any> = createStore(rootReducer, composeEnhancers(applyMiddleware(epicMiddleware)));

epicMiddleware.run(rootEpic);

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
