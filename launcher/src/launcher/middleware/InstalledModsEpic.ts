import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction, receiveInstalledMods } from "../actions";
import { getRweModsPath } from "../util";
import { getInstalledMods } from "../../common/mods";

export const installedModsEpic = (): rx.Observable<AppAction> => {
  const modsPath = getRweModsPath();
  return rx
    .from(getInstalledMods(modsPath))
    .pipe(rxop.map(receiveInstalledMods));
};
