import { toggleItem, moveUp, moveDown } from "../common/util";
import { AppAction } from "./actions";

export interface ModsDialogState {
  activeMods: string[];
  selectedMod?: string;
}

export function modDialogReducer(
  modsDialog: ModsDialogState,
  action: AppAction
): ModsDialogState {
  switch (action.type) {
    case "SELECT_MOD": {
      return { ...modsDialog, selectedMod: action.name };
    }
    case "TOGGLE_MOD": {
      const newList = toggleItem(modsDialog.activeMods, action.name);
      return { ...modsDialog, activeMods: newList };
    }
    case "MOD_UP": {
      if (!modsDialog.selectedMod) {
        return modsDialog;
      }
      const newList = moveUp(modsDialog.activeMods, modsDialog.selectedMod);
      return { ...modsDialog, activeMods: newList };
    }
    case "MOD_DOWN": {
      if (!modsDialog.selectedMod) {
        return modsDialog;
      }
      const newList = moveDown(modsDialog.activeMods, modsDialog.selectedMod);
      return { ...modsDialog, activeMods: newList };
    }

    default:
      return modsDialog;
  }
}
