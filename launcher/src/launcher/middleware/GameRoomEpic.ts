import { ofType, StateObservable } from "redux-observable";
import * as rx from "rxjs";
import * as rxop from "rxjs/operators";
import {
  AppAction,
  closeSelectMapDialog,
  gameEnded,
  LeaveGameAction,
} from "../actions";
import { getRoom, State, InstalledModInfo } from "../state";
import { FilledPlayerSlot, CurrentGameState } from "../gameClient/state";
import * as protocol from "../../game-server/protocol";

import { getIpv4Address } from "../../common/ip-lookup";
import {
  execRwe,
  RweArgs,
  RweArgsEmptyPlayerSlot,
  RweArgsFilledPlayerSlot,
  RweArgsPlayerController,
  RweArgsPlayerInfo,
  RweArgsPlayerSlot,
} from "../rwe";
import { assertNever, masterServer, choose } from "../../common/util";
import { EpicDependencies } from "./EpicDependencies";
import { ReceiveCreateGameResponseAction } from "../masterClient/actions";

function rweArgsFromCurrentGameState(
  installedMods: InstalledModInfo[],
  game: CurrentGameState,
  startInfo: protocol.StartGamePayload
): RweArgs {
  const playersArgs: RweArgsPlayerSlot[] = game.players.map((x, i) => {
    switch (x.state) {
      case "empty":
      case "closed": {
        const s: RweArgsEmptyPlayerSlot = { state: "empty" };
        return s;
      }
      case "filled": {
        const controller: RweArgsPlayerController =
          x.player.id === game.localPlayerId
            ? { type: "human" }
            : {
                type: "remote",
                host: startInfo.addresses.find(
                  ([id, _]) => id === x.player.id
                )![1],
                port: 6670 + i,
              };
        const a: RweArgsPlayerInfo = {
          name: x.player.name,
          side: x.player.side,
          color: x.player.color,
          controller,
        };
        const s: RweArgsFilledPlayerSlot = { ...a, state: "filled" };
        return s;
      }
      default:
        return assertNever(x);
    }
  });
  const portOffset = game.players.findIndex(
    x => x.state === "filled" && x.player.id === game.localPlayerId
  );
  if (game.mapName === undefined) {
    throw new Error("map is not set");
  }
  return {
    dataPaths: choose(game.activeMods, name =>
      installedMods.find(x => x.name === name)
    ).map(x => x.path),
    map: game.mapName,
    port: 6670 + portOffset,
    players: playersArgs,
  };
}

export const gameRoomEpic = (
  action$: rx.Observable<AppAction>,
  state$: StateObservable<State>,
  deps: EpicDependencies
): rx.Observable<AppAction> => {
  const clientService = deps.clientService;
  const masterClientService = deps.masterClentService;

  return action$.pipe(
    rxop.flatMap(action => {
      switch (action.type) {
        case "HOST_GAME_FORM_CONFIRM": {
          masterClientService.requestCreateGame(
            action.gameDescription,
            action.players
          );
          action$
            .pipe(
              ofType<AppAction, ReceiveCreateGameResponseAction>(
                "RECEIVE_CREATE_GAME_RESPONSE"
              ),
              rxop.first()
            )
            .subscribe(x => {
              getIpv4Address().then(clientAddress => {
                clientService.connectToServer(
                  `${masterServer()}/rooms`,
                  x.payload.game_id,
                  action.playerName,
                  clientAddress,
                  state$.value.installedMods!.map(m => m.name),
                  x.payload.admin_key
                );
              });
            });
          break;
        }
        case "JOIN_SELECTED_GAME_CONFIRM": {
          const state = state$.value;
          if (state.selectedGameId === undefined) {
            break;
          }
          const selectedGameId = state.selectedGameId;
          const connectionString = `${masterServer()}/rooms`;
          console.log(`connecting to ${connectionString}`);
          rx.from(getIpv4Address())
            .pipe(
              rxop.takeUntil(
                action$.pipe(ofType<AppAction, LeaveGameAction>("LEAVE_GAME"))
              )
            )
            .subscribe(clientAddress => {
              clientService.connectToServer(
                connectionString,
                selectedGameId,
                action.name,
                clientAddress,
                state.installedMods!.map(m => m.name)
              );
            });
          break;
        }
        case "SEND_CHAT_MESSAGE": {
          clientService.sendChatMessage(action.message);
          break;
        }
        case "CHANGE_SIDE": {
          clientService.changeSide(action.side);
          break;
        }
        case "CHANGE_TEAM": {
          clientService.changeTeam(action.team);
          break;
        }
        case "CHANGE_COLOR": {
          clientService.changeColor(action.color);
          break;
        }
        case "OPEN_SLOT": {
          clientService.openSlot(action.slotId);
          break;
        }
        case "CLOSE_SLOT": {
          clientService.closeSlot(action.slotId);
          break;
        }
        case "REQUEST_SET_ACTIVE_MODS": {
          clientService.setActiveMods(action.mods);
          break;
        }
        case "RECEIVE_ACTIVE_MODS_CHANGED": {
          deps.bridgeService.clearDataPaths();
          const installedMods = state$.value.installedMods;
          if (!installedMods) {
            break;
          }
          const resolvedMods = choose(action.payload.mods, x =>
            installedMods.find(y => y.name === x)
          );
          for (const info of resolvedMods) {
            deps.bridgeService.addDataPath(info.path);
          }
          break;
        }
        case "TOGGLE_READY": {
          const state = state$.value;
          const room = state.currentGame;
          if (!room) {
            break;
          }
          if (room.localPlayerId === undefined) {
            break;
          }
          const localPlayerSlot = room.players.find(
            x => x.state === "filled" && x.player.id === room.localPlayerId
          )! as FilledPlayerSlot;
          const currentValue = localPlayerSlot.player.ready;
          clientService.setReadyState(!currentValue);
          break;
        }
        case "LEAVE_GAME": {
          clientService.disconnect();
          break;
        }
        case "DISCONNECT_GAME": {
          deps.bridgeService.clearDataPaths();
          break;
        }
        case "START_GAME": {
          deps.bridgeService.clearDataPaths();
          break;
        }
        case "SEND_START_GAME": {
          clientService.requestStartGame();
          break;
        }
        case "RECEIVE_START_GAME": {
          const state = state$.value;
          const room = state.currentGame;
          if (!room || !state.installedMods) {
            break;
          }
          return rx
            .from(
              execRwe(
                rweArgsFromCurrentGameState(
                  state.installedMods,
                  room,
                  action.payload
                )
              )
            )
            .pipe(
              rxop.mapTo(undefined),
              rxop.catchError(() => rx.of(undefined)),
              rxop.mapTo(gameEnded())
            );
        }
        case "CHANGE_MAP": {
          const state = state$.value;
          const room = getRoom(state);
          if (!room) {
            break;
          }
          if (!room.mapDialog) {
            break;
          }
          if (!room.mapDialog.selectedMap) {
            break;
          }
          clientService.changeMap(room.mapDialog.selectedMap);
          return rx.of<AppAction>(closeSelectMapDialog());
        }
      }
      return rx.empty();
    })
  );
};
