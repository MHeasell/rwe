import { combineEpics } from "redux-observable";
import { wizardEpic } from "../wizardEpic";
import { masterClientEventsEpic } from "./MasterClientEventsEpic";
import { gameClientEventsEpic } from "./GameClientEventsEpic";
import { gameRoomEpic } from "./GameRoomEpic";
import { launchRweEpic } from "./LaunchRweEpic";
import { rweBridgeEpic } from "./RweBridgeEpic";
import { installedModsEpic } from "./InstalledModsEpic";
import { videoModesEpic } from "./VideoModesEpic";
import { rweConfigEpic } from "./RweConfigEpic";
import { rweConfigDialogEpic } from "./RweConfigDialogEpic";

export const rootEpic = combineEpics(
  masterClientEventsEpic,
  gameClientEventsEpic,
  gameRoomEpic,
  launchRweEpic,
  rweBridgeEpic,
  installedModsEpic,
  wizardEpic,
  videoModesEpic,
  rweConfigEpic,
  rweConfigDialogEpic
);
