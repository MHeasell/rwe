import * as React from "react";
import { hot } from "react-hot-loader";
import { connect } from "react-redux";
import { AppScreen, State } from "../state";
import GameRoomScreen from "./GameRoomScreen";
import HostGameForm from "./HostGameForm";
import OverviewScreen from "./OverviewScreen";

interface ScreenContainerProps {
  screen: AppScreen;
}

function ScreenContainer(props: ScreenContainerProps): JSX.Element  {
  switch (props.screen.screen) {
    case "overview":
      return <OverviewScreen />;
    case "host-form":
      return <HostGameForm />;
    case "game-room":
      return <GameRoomScreen />;
  }
}

function mapStateToProps(state: State): ScreenContainerProps {
  return { screen: state.currentScreen };
}

const ConnectedScreenContainer = connect(mapStateToProps)(ScreenContainer);

function App() {
  return (
    <ConnectedScreenContainer />
  );
}

export default hot(module)(App);
