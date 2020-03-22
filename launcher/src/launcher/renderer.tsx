// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

import CssBaseline from "@material-ui/core/CssBaseline";
import * as React from "react";
import * as ReactDOM from "react-dom";
import { Provider } from "react-redux";
import { applyMiddleware, compose, createStore, Store } from "redux";
import { createEpicMiddleware } from "redux-observable";
import { AppAction, gameEnded } from "./actions";
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
import { execRwe } from "./rwe";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";

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

// eslint-disable-next-line prefer-const
let store: Store<any>;

const executeSideEffect = (se: SideEffect) => {
  switch (se.type) {
    case "LAUNCH_RWE": {
      return rx
        .from(execRwe(se.args))
        .pipe(
          rxop.mapTo(undefined),
          rxop.catchError(() => rx.of(undefined)),
          rxop.mapTo(gameEnded())
        )
        .subscribe(store.dispatch);
    }
  }
};

const composeEnhancers: typeof compose =
  (window as any).__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ || compose;
store = createStore(
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
