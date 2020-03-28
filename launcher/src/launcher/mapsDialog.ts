import { AppAction } from "./actions";

export interface MapDialogState {
  maps?: { name: string; source: string }[];
  selectedMap?: string;
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
      if (mapDialog.selectedMap && mapDialog.selectedMap === action.mapName) {
        return mapDialog;
      }
      return {
        ...mapDialog,
        selectedMap: action.mapName,
      };
    }
    default:
      return mapDialog;
  }
}
