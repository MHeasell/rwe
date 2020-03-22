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
import rootReducer from "./reducers";
import { SideEffect, State } from "./state";
import { GameClientService } from "../game-server/game-client";

import { RweBridge } from "./bridge";
import { MasterClientService } from "../master-server/master-client";
import "./style.css";
import { masterServer } from "../common/util";
import { EpicDependencies } from "./middleware/EpicDependencies";
import { rootEpic } from "./middleware/RootEpic";
import { createEnhancer } from "./sideEffects";

const masterClentService = new MasterClientService();
masterClentService.connectToServer(`${masterServer()}/master`);

const epicDeps = {
  clientService: new GameClientService(),
  masterClentService,
  bridgeService: new RweBridge(),
};

const epicMiddleware = createEpicMiddleware<
  AppAction,
  AppAction,
  State,
  EpicDependencies
>({
  dependencies: epicDeps,
});

const executeSideEffect = (se: SideEffect) => {
  console.log("blah");
};

const composeEnhancers: typeof compose =
  (window as any).__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ || compose;
const store: Store<any> = createStore(
  rootReducer as any, // shhhhh
  composeEnhancers(
    applyMiddleware(epicMiddleware),
    createEnhancer(executeSideEffect)
  )
);

epicMiddleware.run(rootEpic);

ReactDOM.render(
  <React.Fragment>
    <CssBaseline />
    <Provider store={store}>
      <App />
    </Provider>
  </React.Fragment>,
  document.getElementById("app")
);
