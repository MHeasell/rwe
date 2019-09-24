import { AppAction } from "./actions";

export interface SelectedMapDetails {
  description: string;
  memory: string;
  numberOfPlayers: string;
}

export interface SelectedMapInfo {
  name: string;
  minimap?: string;
  details?: SelectedMapDetails;
}

export interface MapDialogState {
  maps?: string[];
  selectedMap?: SelectedMapInfo;
}

export function mapDialogReducer(
  mapDialog: MapDialogState,
  action: AppAction
): MapDialogState {
  switch (action.type) {
    case "RECEIVE_MAP_LIST": {
      return { ...mapDialog, maps: action.maps };
    }
    case "DIALOG_SELECT_MAP": {
      if (
        mapDialog.selectedMap &&
        mapDialog.selectedMap.name === action.mapName
      ) {
        return mapDialog;
      }
      return {
        ...mapDialog,
        selectedMap: { name: action.mapName },
      };
    }
    case "RECEIVE_MINIMAP": {
      if (!mapDialog.selectedMap) {
        return mapDialog;
      }

      const selectedMapInfo = {
        ...mapDialog.selectedMap,
        minimap: action.path,
      };
      return { ...mapDialog, selectedMap: selectedMapInfo };
    }
    case "RECEIVE_MAP_INFO": {
      if (!mapDialog.selectedMap) {
        return mapDialog;
      }

      const selectedMapInfo = {
        ...mapDialog.selectedMap,
        details: action.info,
      };
      return { ...mapDialog, selectedMap: selectedMapInfo };
    }
    default:
      return mapDialog;
  }
}
