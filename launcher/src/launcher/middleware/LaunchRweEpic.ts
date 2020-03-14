import { ofType, StateObservable } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction, gameEnded, LaunchRweAction } from "../actions";
import { State } from "../state";
import { execRwe } from "../rwe";
import { choose } from "../../common/util";
import { EpicDependencies } from "./EpicDependencies";

export const launchRweEpic = (
  action$: rx.Observable<AppAction>,
  state$: StateObservable<State>,
  _: EpicDependencies
): rx.Observable<AppAction> => {
  return action$.pipe(
    ofType<AppAction, LaunchRweAction>("LAUNCH_RWE"),
    rxop.flatMap(() => {
      return rx
        .from(
          execRwe({
            dataPaths: choose(state$.value.activeMods, name =>
              state$.value.installedMods!.find(x => x.name === name)
            ).map(x => x.path),
          })
        )
        .pipe(
          rxop.mapTo(undefined),
          rxop.catchError(() => rx.of(undefined)),
          rxop.mapTo(gameEnded())
        );
    })
  );
};
