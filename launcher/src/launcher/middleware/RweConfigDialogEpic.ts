import { ofType } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction, SubmitSettingsDialogAction } from "../actions";
import { getRweConfigPath } from "../util";
import { writeConfigToFile } from "../rweConfig";

export const rweConfigDialogEpic = (
  action$: rx.Observable<AppAction>
): rx.Observable<AppAction> => {
  const configFilePath = getRweConfigPath();

  return action$.pipe(
    ofType<AppAction, SubmitSettingsDialogAction>("SUBMIT_SETTINGS_DIALOG"),
    rxop.concatMap(action => {
      return rx.from(writeConfigToFile(configFilePath, action.settings));
    }),
    rxop.concatMap(() => rx.empty())
  );
};
