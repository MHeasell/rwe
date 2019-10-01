import { AppAction } from "./actions";

export interface WizardState {
  step: "welcome" | "working" | "success" | "fail";
}

export function wizardReducer(
  state: WizardState,
  action: AppAction
): WizardState {
  switch (action.type) {
    case "wizard/NEXT": {
      return { step: "working" };
    }
    case "wizard/DONE": {
      return { step: "success" };
    }
    case "wizard/FAIL": {
      return { step: "fail" };
    }
    default:
      return state;
  }
}
