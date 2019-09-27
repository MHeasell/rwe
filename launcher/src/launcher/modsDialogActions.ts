export interface SelectModAction {
  type: "SELECT_MOD";
  name: string;
}

export function selectMod(name: string): SelectModAction {
  return {
    type: "SELECT_MOD",
    name,
  };
}

export interface ToggleModAction {
  type: "TOGGLE_MOD";
  name: string;
}

export function toggleMod(name: string): ToggleModAction {
  return {
    type: "TOGGLE_MOD",
    name,
  };
}

export interface ModUpAction {
  type: "MOD_UP";
}

export function modUp(): ModUpAction {
  return {
    type: "MOD_UP",
  };
}

export interface ModDownAction {
  type: "MOD_DOWN";
}

export function modDown(): ModDownAction {
  return {
    type: "MOD_DOWN",
  };
}

export type ModsDialogAction =
  | SelectModAction
  | ToggleModAction
  | ModUpAction
  | ModDownAction;
