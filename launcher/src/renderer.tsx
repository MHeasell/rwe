// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

import CssBaseline from "@material-ui/core/CssBaseline";
import * as React from "react";
import * as ReactDOM from "react-dom";
import { Provider } from "react-redux";
import { applyMiddleware, createStore, Store, compose } from "redux";
import { AppAction } from "./actions";
import App from "./components/App";
import rootReducer from "./reducers";
import { createEpicMiddleware } from "redux-observable";
import { State } from "./state";
import { rootEpic, EpicDependencies } from "./middleware/GameRoomEpic";
import { GameClientService } from "./ws/game-client";

import "./style.css";
import { MasterClientService } from "./master/master-client";

const masterClentService = new MasterClientService();
masterClentService.connectToServer("http://127.0.0.1:5000/master");
const epicMiddleware = createEpicMiddleware<AppAction, AppAction, State, EpicDependencies>({
  dependencies: { clientService: new GameClientService(), masterClentService },
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
