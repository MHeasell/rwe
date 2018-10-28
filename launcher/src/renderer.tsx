// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

import CssBaseline from "@material-ui/core/CssBaseline";
import * as React from "react";
import * as ReactDOM from "react-dom";
import { Provider } from "react-redux";
import { applyMiddleware, compose, createStore, Store } from "redux";
import { createEpicMiddleware } from "redux-observable";
import { AppAction } from "./actions";
import App from "./components/App";
import { EpicDependencies, rootEpic } from "./middleware/GameRoomEpic";
import rootReducer from "./reducers";
import { State } from "./state";
import { GameClientService } from "./ws/game-client";

import { RweBridge } from "./bridge";
import { MasterClientService } from "./master/master-client";
import "./style.css";
import { masterServer } from "./util";

const masterClentService = new MasterClientService();
masterClentService.connectToServer(`${masterServer()}/master`);
const epicMiddleware = createEpicMiddleware<AppAction, AppAction, State, EpicDependencies>({
  dependencies: { clientService: new GameClientService(), masterClentService, bridgeService: new RweBridge() },
});

const composeEnhancers = (window as any).__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ || compose;
const store: Store<any> = createStore(rootReducer, composeEnhancers(applyMiddleware(epicMiddleware)));

epicMiddleware.run(rootEpic);

ReactDOM.render(
  <React.Fragment>
    <CssBaseline />
    <Provider store={store}>
      <App />
    </Provider>
  </React.Fragment>,
  document.getElementById("app"));
