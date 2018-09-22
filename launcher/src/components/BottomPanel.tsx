import Button from "@material-ui/core/Button";
import { createStyles, Theme, withStyles, WithStyles } from "@material-ui/core/styles";
import * as React from "react";
import { connect } from "react-redux";
import { Dispatch } from "redux";
import { hostGame, launchRwe, joinSelectedGame } from "../actions";
import { canHostGame, canJoinSelectedGame, canLaunchRwe, State } from "../state";

const styles = (theme: Theme) => createStyles({
  joinGameButton: {
    "margin-left": theme.spacing.unit,
  },
});

interface BottomPanelStateProps {
  hostEnabled: boolean;
  joinEnabled: boolean;
  launchEnabled: boolean;
}

interface BottomPanelDispatchProps {
  onHostGame: () => void;
  onJoinGame: () => void;
  onLaunchRwe: () => void;
}

interface UnstyledBottomPanelProps extends BottomPanelStateProps, BottomPanelDispatchProps {}
interface BottomPanelProps extends UnstyledBottomPanelProps, WithStyles<typeof styles> {}

const UnstyledBottomPanel = (props: BottomPanelProps) => {
  return (
    <div className="bottom-panel">
      <div className="bottom-panel-left">
        <Button variant="contained" disabled={!props.hostEnabled} onClick={props.onHostGame}>Host Game</Button>
        <Button variant="contained" color="primary" className={props.classes.joinGameButton} disabled={!props.joinEnabled} onClick={props.onJoinGame}>Join Game</Button>
      </div>
      <div className="bottom-panel-right">
        <Button variant="contained" disabled={!props.launchEnabled} onClick={props.onLaunchRwe}>Launch RWE</Button>
      </div>
    </div>
  );
};

function mapStateToProps(state: State): BottomPanelStateProps {
  return {
    hostEnabled: canHostGame(state),
    joinEnabled: canJoinSelectedGame(state),
    launchEnabled: canLaunchRwe(state),
  };
}

function mapDispatchToProps(dispatch: Dispatch): BottomPanelDispatchProps {
  return {
    onHostGame: () => dispatch(hostGame()),
    onJoinGame: () => dispatch(joinSelectedGame()),
    onLaunchRwe: () => dispatch(launchRwe()),
  };
}

const UnconnectedBottomPanel = withStyles(styles)(UnstyledBottomPanel);
export default connect(mapStateToProps, mapDispatchToProps)(UnconnectedBottomPanel);
