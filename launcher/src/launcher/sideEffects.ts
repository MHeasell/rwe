import {
  Store,
  StoreEnhancer,
  DeepPartial,
  Reducer,
  Action,
  Dispatch,
} from "redux";

export type StateWithSideEffects<S, SE> = {
  isStateWithSideEffects: true;
  state: S;
  sideEffects: SE[];
};

export type SideEffectExecutor<SE> = (effect: SE) => void;

const withSideEffects = <S, SE>(
  state: S,
  ...sideEffects: SE[]
): StateWithSideEffects<S, SE> => ({
  isStateWithSideEffects: true,
  state,
  sideEffects,
});

const isLiftedState = <S, SE>(s: any): s is StateWithSideEffects<S, SE> =>
  s && s.isStateAndSideEffects === true;

const liftState = <S, SE>(
  state: S | StateWithSideEffects<S, SE>
): StateWithSideEffects<S, SE> =>
  isLiftedState(state) ? state : withSideEffects(state);

type EnhancedReducer<S, SE, A extends Action> = (
  state: S | undefined,
  action: A
) => S | StateWithSideEffects<S, SE>;

export const createEnhancer = <SE>(executor: SideEffectExecutor<SE>) => {
  const myStoreEnhancer: StoreEnhancer = createStore => <S, A extends Action>(
    reducer: EnhancedReducer<S, SE, A>,
    initialState?: DeepPartial<S>
  ): Store<S, A> => {
    let sideEffects: SE[] = [];

    const wrapReducer = (
      reducer: (
        state: S | undefined,
        action: A
      ) => S | StateWithSideEffects<S, SE>
    ): Reducer<S, A> => (state: S | undefined, action: A) => {
      const newState = liftState(reducer(state, action));
      sideEffects.push(...newState.sideEffects);
      return newState.state;
    };

    const store = createStore(wrapReducer(reducer), initialState);

    const dispatch: Dispatch<A> = <T extends A>(action: T) => {
      const result = store.dispatch(action);
      for (const sideEffect of sideEffects) {
        executor(sideEffect);
      }
      sideEffects = [];
      return result;
    };

    const replaceReducer = (reducer: Reducer<S, A>) =>
      store.replaceReducer(wrapReducer(reducer));

    return {
      ...store,
      dispatch,
      replaceReducer,
    };
  };
  return myStoreEnhancer;
};

export const composeReducers = <S, SE, A extends Action>(
  a: EnhancedReducer<S, SE, A>,
  b: EnhancedReducer<S, SE, A>
): EnhancedReducer<S, SE, A> => (state: S | undefined, action: A) => {
  const resultB = liftState(b(state, action));
  const resultA = liftState(a(resultB.state, action));
  return withSideEffects(
    resultA.state,
    ...resultB.sideEffects,
    ...resultA.sideEffects
  );
};

export const reduceReducers = <S, SE, A extends Action>(
  ...reducers: EnhancedReducer<S, SE, A>[]
) => reducers.reduce(composeReducers);
