import { StateObservable, ofType } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import fs from "fs";
import { AppAction } from "./actions";
import { State } from "./state";
import { EpicDependencies } from "./middleware/GameRoomEpic";
import { NextAction, done, fail, open } from "./wizardActions";
import {
  automaticallySetUpTaMod,
  getRweModsPath,
  automaticallySetUpTaModManual,
} from "./util";
import path from "path";

export function wizardEpic(
  action$: rx.Observable<AppAction>,
  state$: StateObservable<State>,
  deps: EpicDependencies
): rx.Observable<AppAction> {
  const modsPath = getRweModsPath();
  const taModPath = path.join(modsPath, "ta");

  const stream = action$.pipe(
    ofType<AppAction, NextAction>("wizard/NEXT"),
    rxop.concatMap(x =>
      rx.from(
        x.path
          ? automaticallySetUpTaModManual(taModPath, [x.path])
          : automaticallySetUpTaMod(taModPath)
      )
    ),
    rxop.map(x => (x ? done() : fail()))
  );

  const initialEvent = rx
    .from(
      new Promise<boolean>(resolve => {
        fs.exists(taModPath, resolve);
      })
    )
    .pipe(
      rxop.filter(x => !x),
      rxop.mapTo(open())
    );

  return rx.merge(initialEvent, stream);
}
