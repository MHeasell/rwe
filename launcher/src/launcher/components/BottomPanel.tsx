import Button from "@material-ui/core/Button";
import {
  createStyles,
  Theme,
  withStyles,
  WithStyles,
} from "@material-ui/core/styles";
import * as React from "react";
import { Grid } from "@material-ui/core";

const styles = (theme: Theme) =>
  createStyles({
    joinGameButton: {
      "margin-left": theme.spacing(1),
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
  onOpenModsDialog: () => void;
  onOpenConfigDialog: () => void;
}

interface UnstyledBottomPanelProps
  extends BottomPanelStateProps,
    BottomPanelDispatchProps {}
interface BottomPanelProps
  extends UnstyledBottomPanelProps,
    WithStyles<typeof styles> {}

const UnstyledBottomPanel = (props: BottomPanelProps) => {
  return (
    <div className="bottom-panel">
      <div className="bottom-panel-left">
        <Button
          variant="contained"
          disabled={!props.hostEnabled}
          onClick={props.onHostGame}
        >
          Host Game
        </Button>
        <Button
          variant="contained"
          color="primary"
          className={props.classes.joinGameButton}
          disabled={!props.joinEnabled}
          onClick={props.onJoinGame}
        >
          Join Game
        </Button>
      </div>
      <div className="bottom-panel-right">
        <Grid container spacing={1}>
          <Grid item>
            <Button variant="contained" onClick={props.onOpenConfigDialog}>
              RWE Settings
            </Button>
          </Grid>
          <Grid item>
            <Button variant="contained" onClick={props.onOpenModsDialog}>
              Single Player Mods
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="contained"
              disabled={!props.launchEnabled}
              onClick={props.onLaunchRwe}
            >
              Launch RWE
            </Button>
          </Grid>
        </Grid>
      </div>
    </div>
  );
};

const BottomPanel = withStyles(styles)(UnstyledBottomPanel);
export default BottomPanel;
