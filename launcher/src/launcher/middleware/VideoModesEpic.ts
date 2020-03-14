import { StateObservable } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction, receiveVideoModes } from "../actions";
import { State } from "../state";

import { EpicDependencies } from "./EpicDependencies";

export const videoModesEpic = (
  action$: rx.Observable<AppAction>,
  state$: StateObservable<State>,
  deps: EpicDependencies
): rx.Observable<AppAction> => {
  return rx
    .from(deps.bridgeService.getVideoModes())
    .pipe(rxop.map(x => receiveVideoModes(x.modes)));
};
