import * as React from "react";
import { connect } from "react-redux";
import { Dispatch } from "redux";
import BottomPanel from "./BottomPanel";
import GamesTable from "./GamesTable";
import { PlayerNameDialog } from "./PlayerNameDialog";
import { State } from "../state";
import { joinSelectedGameCancel, joinSelectedGameConfirm } from "../actions";
import { Paper, Typography, Divider, Slide, Collapse } from "@material-ui/core";

function MainPanel() {
  return (
    <div className="main-panel">
      <Typography variant="h6" className="main-panel-title">Online Games</Typography>
      <GamesTable />
    </div>
  );
}

interface ConnectionNoticeProps {
  connected: boolean;
}

function ConnectionNotice(props: ConnectionNoticeProps) {
  return (
    <React.Fragment>
      <Collapse in={!props.connected}>
        <div className="connection-notice">
          <Typography>
            Cannot reach the master server, attempting to reconnect...
          </Typography>
        </div>
      </Collapse>
      <Divider />
    </React.Fragment>
  );
}

interface OverviewScreenDispatchProps {
  onDialogClose: () => void;
  onDialogConfirm: (name: string) => void;
}

interface OverviewScreenStateProps {
  connected: boolean;
  dialogOpen: boolean;
}

interface OverviewScreenProps extends OverviewScreenDispatchProps, OverviewScreenStateProps {}

function OverviewScreen(props: OverviewScreenProps) {
  return (
    <Paper className="app-container">
      <ConnectionNotice connected={props.connected} />
      <MainPanel />
      <BottomPanel />
      <PlayerNameDialog open={props.dialogOpen} onConfirm={props.onDialogConfirm} onClose={props.onDialogClose} />
    </Paper>
  );
}

function mapStateToProps(state: State): OverviewScreenStateProps {
  const dialogOpen = state.currentScreen.screen === "overview" ? state.currentScreen.dialogOpen : false;
  return { connected: state.masterServerConnectionStatus === "connected", dialogOpen };
}

function mapDispatchToProps(dispatch: Dispatch): OverviewScreenDispatchProps {
  return {
    onDialogClose: () => { dispatch(joinSelectedGameCancel()); },
    onDialogConfirm: (name: string) => { dispatch(joinSelectedGameConfirm(name)); },
  };
}

export default connect(mapStateToProps, mapDispatchToProps)(OverviewScreen);
