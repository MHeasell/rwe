import { MapDialogState } from "./mapsDialog";
import { WizardState } from "./wizard";
import { RweConfig } from "./rweConfig";
import { CurrentGameState } from "./gameClient/state";
import { isFull, MasterClientState } from "./masterClient/state";
import { StateWithSideEffects } from "./sideEffects";
import { RweArgs } from "./rwe";

export interface OverviewScreen {
  screen: "overview";
}

export interface HostFormScreen {
  screen: "host-form";
}

export interface GameRoomScreen {
  screen: "game-room";
  room?: GameRoom;
}

export interface MapCacheValue {
  description: string;
  memory: string;
  numberOfPlayers: string;
  minimap?: string;
}

export interface GameRoom {
  mapDialog?: MapDialogState;
  mapCache: { [key: string]: MapCacheValue };
}

export type AppScreen = HostFormScreen | OverviewScreen | GameRoomScreen;

export function getRoom(state: State): GameRoom | undefined {
  if (state.currentScreen.screen !== "game-room") {
    return undefined;
  }
  return state.currentScreen.room;
}

export interface InstalledModInfo {
  name: string;
  path: string;
}

export interface VideoMode {
  width: number;
  height: number;
}

export interface State {
  installedMods?: InstalledModInfo[];
  videoModes?: VideoMode[];
  activeMods: string[];
  selectedGameId?: number;
  currentGame?: CurrentGameState;
  currentScreen: AppScreen;
  isRweRunning: boolean;
  wizard?: WizardState;
  rweConfig?: RweConfig;
  masterClient: MasterClientState;
}

export type SideEffect = { type: "LAUNCH_RWE"; args?: RweArgs };

export type StateAndSideEffects = StateWithSideEffects<State, SideEffect>;

export type WithSideEffects<S> = StateWithSideEffects<S, SideEffect>;

export function canJoinSelectedGame(state: State): boolean {
  if (state.isRweRunning) {
    return false;
  }

  if (state.selectedGameId === undefined) {
    return false;
  }

  const game = state.masterClient.games.find(
    g => g.id === state.selectedGameId
  );
  if (!game || isFull(game)) {
    return false;
  }

  return true;
}

export function canHostGame(state: State): boolean {
  return true;
}

export function canLaunchRwe(state: State): boolean {
  return !state.isRweRunning;
}
