import { StateObservable } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import { AppAction, receiveMapList, receiveCombinedMapInfo } from "../actions";
import { State, getRoom } from "../state";

import { chooseOp } from "../../common/rxutil";
import { EpicDependencies } from "./EpicDependencies";
import { createSelector } from "reselect";

function getSelectedMap(state: State) {
  const room = getRoom(state);
  if (!room || !room.mapDialog || !room.mapDialog.selectedMap) {
    return undefined;
  }
  const name = room.mapDialog.selectedMap;
  return room.mapDialog.maps?.find(x => x.name == name);
}

function getSelectedMapCacheEntry(state: State) {
  const room = getRoom(state);
  if (!room) {
    return undefined;
  }
  return room.mapDialog && room.mapDialog.selectedMap
    ? room.mapCache[room.mapDialog.selectedMap]
    : undefined;
}

const mapNameAndCacheSelector = createSelector(
  getSelectedMap,
  getSelectedMapCacheEntry,
  (mapName, cacheEntry) => [mapName, cacheEntry] as const
);

export const rweBridgeEpic = (
  action$: rx.Observable<AppAction>,
  state$: StateObservable<State>,
  deps: EpicDependencies
): rx.Observable<AppAction> => {
  const selectedMap$ = state$.pipe(
    rxop.map(mapNameAndCacheSelector),
    chooseOp(([mapName, cacheEntry]) =>
      mapName && !cacheEntry ? mapName : undefined
    )
  );

  const mapInfoStream = selectedMap$.pipe(
    rxop.switchMap(
      (mapListItem): rx.Observable<AppAction> => {
        const mapInfo$ = rx.from(
          deps.bridgeService.getMapInfo(mapListItem.source, mapListItem.name)
        );
        const minimap$ = rx.from(
          deps.bridgeService.getMinimap(mapListItem.source, mapListItem.name)
        );
        return rx.combineLatest([mapInfo$, minimap$]).pipe(
          rxop.map(([infoResponse, minimapResponse]) => {
            return receiveCombinedMapInfo(
              mapListItem.name,
              infoResponse,
              minimapResponse.path
            );
          })
        );
      }
    )
  );

  const actionPipe = action$.pipe(
    rxop.flatMap(
      (action): rx.Observable<AppAction> => {
        switch (action.type) {
          case "OPEN_SELECT_MAP_DIALOG": {
            return rx
              .from(deps.bridgeService.getMapList())
              .pipe(rxop.map(x => receiveMapList(x.maps)));
          }
        }
        return rx.empty();
      }
    )
  );

  return rx.merge(mapInfoStream, actionPipe);
};
