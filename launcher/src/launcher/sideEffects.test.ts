import { isLiftedState, liftState } from "./sideEffects";

describe("isLiftedState", () => {
  it("returns true when state is lifted", () => {
    const f = liftState("foo");
    expect(isLiftedState(f)).toBe(true);
  });
  it("returns false when state is not lifted", () => {
    const f = "foo";
    expect(isLiftedState(f)).toBe(false);
  });
});
