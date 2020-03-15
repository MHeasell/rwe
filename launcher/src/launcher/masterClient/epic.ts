import { StateObservable } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction } from "../actions";
import {
  masterServerConnect,
  masterServerDisconnect,
  receiveCreateGameResponse,
  receiveGameCreated,
  receiveGameDeleted,
  receiveGameUpdated,
  receiveRooms,
} from "./actions";
import { State } from "../state";
import { EpicDependencies } from "../middleware/EpicDependencies";

export const masterClientEventsEpic = (
  action$: rx.Observable<AppAction>,
  state$: StateObservable<State>,
  deps: EpicDependencies
): rx.Observable<AppAction> => {
  const s = deps.masterClentService;
  return rx.merge<AppAction>(
    s.onConnect.pipe(rxop.map(masterServerConnect)),
    s.onDisconnect.pipe(rxop.map(masterServerDisconnect)),
    s.onGetGamesResponse.pipe(rxop.map(receiveRooms)),
    s.onCreateGameResponse.pipe(rxop.map(receiveCreateGameResponse)),
    s.onGameCreated.pipe(rxop.map(receiveGameCreated)),
    s.onGameUpdated.pipe(rxop.map(receiveGameUpdated)),
    s.onGameDeleted.pipe(rxop.map(receiveGameDeleted))
  );
};
