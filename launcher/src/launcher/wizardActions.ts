export interface NextAction {
  type: "wizard/NEXT";
  path?: string;
}

export function next(path?: string): NextAction {
  return {
    type: "wizard/NEXT",
    path,
  };
}

export interface DoneAction {
  type: "wizard/DONE";
}

export function done(): DoneAction {
  return {
    type: "wizard/DONE",
  };
}

export interface FailAction {
  type: "wizard/FAIL";
}

export function fail(): FailAction {
  return {
    type: "wizard/FAIL",
  };
}

export interface OpenAction {
  type: "wizard/OPEN";
}

export function open(): OpenAction {
  return { type: "wizard/OPEN" };
}

export interface CloseAction {
  type: "wizard/CLOSE";
}

export function close(): CloseAction {
  return {
    type: "wizard/CLOSE",
  };
}

export type WizardAction =
  | NextAction
  | DoneAction
  | FailAction
  | OpenAction
  | CloseAction;
