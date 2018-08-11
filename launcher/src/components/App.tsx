import Button from "@material-ui/core/Button";
import { createStyles, Theme, withStyles, WithStyles } from "@material-ui/core/styles";
import * as React from "react";
import { hot } from "react-hot-loader";
import { connect } from "react-redux";
import { Screen, State } from "../reducers";
import HostGameForm from "./HostGameForm";
import OverviewScreen from "./OverviewScreen";
import GameRoomScreen from "./GameRoomScreen";

interface ScreenContainerProps {
  screen: Screen;
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

const ConnectedScreenContainer = connect(mapStateToProps, undefined)(ScreenContainer);

function App() {
  return (
    <ConnectedScreenContainer />
  );
}

export default hot(module)(App);
