export interface DialogSelectMapAction {
  type: "DIALOG_SELECT_MAP";
  mapName: string;
}

export function dialogSelectMap(mapName: string): DialogSelectMapAction {
  return {
    type: "DIALOG_SELECT_MAP",
    mapName,
  };
}

export type MapsDialogAction = DialogSelectMapAction;
