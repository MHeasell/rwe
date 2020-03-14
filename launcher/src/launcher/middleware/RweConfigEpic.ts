import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction, receiveRweConfig } from "../actions";
import { getRweConfigPath } from "../util";
import { readConfigFromFile } from "../rweConfig";

export const rweConfigEpic = (): rx.Observable<AppAction> => {
  const configFilePath = getRweConfigPath();
  return rx
    .from(readConfigFromFile(configFilePath))
    .pipe(rxop.map(x => receiveRweConfig(x)));
};
