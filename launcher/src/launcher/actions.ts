import { GetMapInfoResponse } from "./bridge";
import { InstalledModInfo, VideoMode } from "./state";
import { MapsDialogAction } from "./mapsDialogActions";
import { WizardAction } from "./wizardActions";
import { RweConfig } from "./rweConfig";
import { GameClientAction } from "./gameClient/actions";
import { PlayerSide } from "./gameClient/state";
import { MasterClientAction } from "./masterClient/actions";

export interface SelectGameAction {
  type: "SELECT_GAME";
  gameId: number;
}

export function selectGame(gameId: number): SelectGameAction {
  return {
    type: "SELECT_GAME",
    gameId,
  };
}

export interface JoinSelectedGameConfirmAction {
  type: "JOIN_SELECTED_GAME_CONFIRM";
  name: string;
}

export function joinSelectedGameConfirm(
  name: string
): JoinSelectedGameConfirmAction {
  return {
    type: "JOIN_SELECTED_GAME_CONFIRM",
    name,
  };
}

export interface HostGameAction {
  type: "HOST_GAME";
}

export function hostGame(): HostGameAction {
  return {
    type: "HOST_GAME",
  };
}

export interface HostGameFormCancelAction {
  type: "HOST_GAME_FORM_CANCEL";
}

export function hostGameFormCancel(): HostGameFormCancelAction {
  return {
    type: "HOST_GAME_FORM_CANCEL",
  };
}

export interface HostGameFormConfirmAction {
  type: "HOST_GAME_FORM_CONFIRM";
  playerName: string;
  gameDescription: string;
  players: number;
}

export function hostGameFormConfirm(
  playerName: string,
  gameDescription: string,
  players: number
): HostGameFormConfirmAction {
  return {
    type: "HOST_GAME_FORM_CONFIRM",
    playerName,
    gameDescription,
    players,
  };
}

export interface ChangeSinglePlayerModsAction {
  type: "CHANGE_SINGLE_PLAYER_MODS";
  newMods?: string[];
}

export function changeSinglePlayerMods(
  newMods?: string[]
): ChangeSinglePlayerModsAction {
  return {
    type: "CHANGE_SINGLE_PLAYER_MODS",
    newMods,
  };
}

export interface LaunchRweAction {
  type: "LAUNCH_RWE";
}

export function launchRwe(): LaunchRweAction {
  return {
    type: "LAUNCH_RWE",
  };
}

export interface LaunchRweEndAction {
  type: "LAUNCH_RWE_END";
}

export function launchRweEnd(): LaunchRweEndAction {
  return {
    type: "LAUNCH_RWE_END",
  };
}

export interface SendChatMessageAction {
  type: "SEND_CHAT_MESSAGE";
  message: string;
}

export function sendChatMessage(message: string): SendChatMessageAction {
  return {
    type: "SEND_CHAT_MESSAGE",
    message,
  };
}

export interface LeaveGameAction {
  type: "LEAVE_GAME";
}

export function leaveGame(): LeaveGameAction {
  return {
    type: "LEAVE_GAME",
  };
}

export interface OpenSlotAction {
  type: "OPEN_SLOT";
  slotId: number;
}

export function openSlot(slotId: number): OpenSlotAction {
  return {
    type: "OPEN_SLOT",
    slotId,
  };
}

export interface CloseSlotAction {
  type: "CLOSE_SLOT";
  slotId: number;
}

export function closeSlot(slotId: number): CloseSlotAction {
  return {
    type: "CLOSE_SLOT",
    slotId,
  };
}

export interface ToggleReadyAction {
  type: "TOGGLE_READY";
}

export function toggleReady(): ToggleReadyAction {
  return {
    type: "TOGGLE_READY",
  };
}

export interface ChangeSideAction {
  type: "CHANGE_SIDE";
  side: PlayerSide;
}

export function changeSide(side: PlayerSide): ChangeSideAction {
  return {
    type: "CHANGE_SIDE",
    side,
  };
}

export interface ChangeTeamAction {
  type: "CHANGE_TEAM";
  team?: number;
}

export function changeTeam(team?: number): ChangeTeamAction {
  return {
    type: "CHANGE_TEAM",
    team,
  };
}

export interface ChangeColorAction {
  type: "CHANGE_COLOR";
  color: number;
}

export function changeColor(color: number): ChangeColorAction {
  return {
    type: "CHANGE_COLOR",
    color,
  };
}

export interface StartGameAction {
  type: "START_GAME";
}

export function startGame(): StartGameAction {
  return {
    type: "START_GAME",
  };
}

export interface GameEndedAction {
  type: "GAME_ENDED";
}

export function gameEnded(): GameEndedAction {
  return {
    type: "GAME_ENDED",
  };
}

export interface SendStartGameAction {
  type: "SEND_START_GAME";
}

export function sendStartGame(): SendStartGameAction {
  return {
    type: "SEND_START_GAME",
  };
}

export interface AddDataPathAction {
  type: "ADD_DATA_PATH";
  path: string;
}

export function addDataPath(path: string): AddDataPathAction {
  return {
    type: "ADD_DATA_PATH",
    path,
  };
}

export interface ClearDataPathsAction {
  type: "CLEAR_DATA_PATHS";
}

export function clearDataPaths(): ClearDataPathsAction {
  return {
    type: "CLEAR_DATA_PATHS",
  };
}

export interface GetMapInfoAction {
  type: "GET_MAP_INFO";
  map: string;
}

export function getMapInfo(map: string): GetMapInfoAction {
  return {
    type: "GET_MAP_INFO",
    map,
  };
}

export interface GetMapListAction {
  type: "GET_MAP_LIST";
}

export function getMapList(): GetMapListAction {
  return {
    type: "GET_MAP_LIST",
  };
}

export interface OpenSelectMapDialogAction {
  type: "OPEN_SELECT_MAP_DIALOG";
}

export function openSelectMapDialog(): OpenSelectMapDialogAction {
  return {
    type: "OPEN_SELECT_MAP_DIALOG",
  };
}

export interface SetActiveModsAction {
  type: "REQUEST_SET_ACTIVE_MODS";
  mods: string[];
}

export function setActiveMods(mods: string[]): SetActiveModsAction {
  return {
    type: "REQUEST_SET_ACTIVE_MODS",
    mods,
  };
}

export interface CloseSelectMapDialogAction {
  type: "CLOSE_SELECT_MAP_DIALOG";
}

export function closeSelectMapDialog(): CloseSelectMapDialogAction {
  return {
    type: "CLOSE_SELECT_MAP_DIALOG",
  };
}

export interface ReceiveMapListAction {
  type: "RECEIVE_MAP_LIST";
  maps: { name: string; source: string }[];
}

export function receiveMapList(
  maps: { name: string; source: string }[]
): ReceiveMapListAction {
  return {
    type: "RECEIVE_MAP_LIST",
    maps,
  };
}

export interface ChangeMapAction {
  type: "CHANGE_MAP";
}

export function changeMap(): ChangeMapAction {
  return {
    type: "CHANGE_MAP",
  };
}

export interface ReceiveCombinedMapInfoAction {
  type: "RECEIVE_COMBINED_MAP_INFO";
  name: string;
  info: GetMapInfoResponse;
  minimapPath: string;
}

export function receiveCombinedMapInfo(
  name: string,
  info: GetMapInfoResponse,
  minimapPath: string
): ReceiveCombinedMapInfoAction {
  return {
    type: "RECEIVE_COMBINED_MAP_INFO",
    name,
    info,
    minimapPath,
  };
}

export interface ReceiveInstalledMods {
  type: "RECEIVE_INSTALLED_MODS";
  mods: InstalledModInfo[];
}

export function receiveInstalledMods(
  mods: InstalledModInfo[]
): ReceiveInstalledMods {
  return {
    type: "RECEIVE_INSTALLED_MODS",
    mods,
  };
}

export interface ReceiveVideoModes {
  type: "RECEIVE_VIDEO_MODES";
  modes: VideoMode[];
}

export function receiveVideoModes(modes: VideoMode[]): ReceiveVideoModes {
  return {
    type: "RECEIVE_VIDEO_MODES",
    modes,
  };
}

export interface SubmitSettingsDialogAction {
  type: "SUBMIT_SETTINGS_DIALOG";
  settings: RweConfig;
}

export function submitSettingsDialog(
  settings: RweConfig
): SubmitSettingsDialogAction {
  return {
    type: "SUBMIT_SETTINGS_DIALOG",
    settings,
  };
}

export interface ReceiveRweConfigAction {
  type: "RECEIVE_RWE_CONFIG";
  settings: RweConfig;
}

export function receiveRweConfig(settings: RweConfig): ReceiveRweConfigAction {
  return {
    type: "RECEIVE_RWE_CONFIG",
    settings,
  };
}

export type AppAction =
  | SelectGameAction
  | JoinSelectedGameConfirmAction
  | HostGameAction
  | HostGameFormCancelAction
  | HostGameFormConfirmAction
  | LaunchRweAction
  | LaunchRweEndAction
  | SendChatMessageAction
  | ChangeSideAction
  | ChangeTeamAction
  | ChangeColorAction
  | OpenSlotAction
  | CloseSlotAction
  | ToggleReadyAction
  | SendStartGameAction
  | LeaveGameAction
  | StartGameAction
  | GameEndedAction
  | AddDataPathAction
  | ClearDataPathsAction
  | GetMapListAction
  | GetMapInfoAction
  | OpenSelectMapDialogAction
  | CloseSelectMapDialogAction
  | ReceiveMapListAction
  | ChangeMapAction
  | ReceiveCombinedMapInfoAction
  | SetActiveModsAction
  | ReceiveInstalledMods
  | ReceiveVideoModes
  | ReceiveRweConfigAction
  | SubmitSettingsDialogAction
  | MapsDialogAction
  | WizardAction
  | ChangeSinglePlayerModsAction
  | GameClientAction
  | MasterClientAction;
