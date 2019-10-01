import * as rx from "rxjs";
import * as rxop from "rxjs/operators";

export function chooseOp<T, U>(
  f: (arg: T) => U | undefined
): rx.OperatorFunction<T, U> {
  const selector = (x: T): rx.Observable<U> => {
    const r = f(x);
    if (r === undefined) {
      return rx.empty();
    }
    return rx.of(r);
  };
  return rxop.concatMap(selector);
}
