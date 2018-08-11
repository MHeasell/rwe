import * as React from "react";
import { connect } from "react-redux";
import { Dispatch } from "redux";
import BottomPanel from "./BottomPanel";
import GamesTable from "./GamesTable";
import { PlayerNameDialog } from "./PlayerNameDialog";
import { State } from "../reducers";
import { joinSelectedGameCancel, joinSelectedGameConfirm } from "../actions";

function MainPanel() {
  return (
    <div className="main-panel">
      <GamesTable />
    </div>
  );
}

interface OverviewScreenDispatchProps {
  onDialogClose: () => void;
  onDialogConfirm: (name: string) => void;
}

interface OverviewScreenStateProps {
  dialogOpen: boolean;
}

interface OverviewScreenProps extends OverviewScreenDispatchProps, OverviewScreenStateProps {}

function OverviewScreen(props: OverviewScreenProps) {
  return (
    <div className="app-container">
      <MainPanel />
      <BottomPanel />
      <PlayerNameDialog open={props.dialogOpen} onConfirm={props.onDialogConfirm} onClose={props.onDialogClose} />
    </div>
  );
}

function mapStateToProps(state: State): OverviewScreenStateProps {
  const dialogOpen = state.currentScreen.screen === "overview" ? state.currentScreen.dialogOpen : false;
  return {
    dialogOpen
  };
}

function mapDispatchToProps(dispatch: Dispatch): OverviewScreenDispatchProps {
  return {
    onDialogClose: () => { dispatch(joinSelectedGameCancel()); },
    onDialogConfirm: (name: string) => { dispatch(joinSelectedGameConfirm(name)); },
  };
}

export default connect(mapStateToProps, mapDispatchToProps)(OverviewScreen);
