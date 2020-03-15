import { Collapse, Divider, Paper, Typography } from "@material-ui/core";
import * as React from "react";
import { connect } from "react-redux";
import { Dispatch } from "redux";
import {
  joinSelectedGameConfirm,
  hostGame,
  launchRwe,
  changeSinglePlayerMods,
  submitSettingsDialog,
} from "../actions";
import {
  State,
  canHostGame,
  canJoinSelectedGame,
  canLaunchRwe,
  VideoMode,
} from "../state";
import BottomPanel from "./BottomPanel";
import GamesTable from "./GamesTable";
import { PlayerNameDialog } from "./PlayerNameDialog";
import { Wizard } from "./Wizard";
import { next, close } from "../wizardActions";
import { WizardState } from "../wizard";
import { SelectModsDialog } from "./SelectModsDialog";
import { RweOptionsDialog } from "./RweOptionsDialog";
import { RweConfig } from "../rweConfig";

function MainPanel() {
  return (
    <div className="main-panel">
      <Typography variant="h6" className="main-panel-title">
        Online Games
      </Typography>
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
  onDialogConfirm: (name: string) => void;
  onWizardNext: (path?: string) => void;
  onWizardClose: () => void;
  onHostGame: () => void;
  onLaunchRwe: () => void;
  onChangeMods: (newMods: string[]) => void;
  onChangeConfig: (newConfig: RweConfig) => void;
}

interface OverviewScreenStateProps {
  installedMods: string[];
  activeMods: string[];
  connected: boolean;
  wizardState?: WizardState;
  hostEnabled: boolean;
  joinEnabled: boolean;
  launchEnabled: boolean;
  videoModes?: VideoMode[];
  rweConfig?: RweConfig;
}

interface OverviewScreenProps
  extends OverviewScreenDispatchProps,
    OverviewScreenStateProps {}

function OverviewScreen(props: OverviewScreenProps) {
  const [modsDialogOpen, setModsDialogOpen] = React.useState(false);
  const [configDialogOpen, setConfigDialogOpen] = React.useState(false);
  const [playerNameDialogOpen, setPlayerNameDialogOpen] = React.useState(false);
  return (
    <>
      <Paper className="app-container">
        <ConnectionNotice connected={props.connected} />
        <MainPanel />
        <BottomPanel
          hostEnabled={props.hostEnabled}
          joinEnabled={props.joinEnabled}
          launchEnabled={props.launchEnabled}
          onHostGame={props.onHostGame}
          onJoinGame={() => setPlayerNameDialogOpen(true)}
          onLaunchRwe={props.onLaunchRwe}
          onOpenModsDialog={() => setModsDialogOpen(true)}
          onOpenConfigDialog={() => setConfigDialogOpen(true)}
        />
        <PlayerNameDialog
          open={playerNameDialogOpen}
          onConfirm={name => {
            setPlayerNameDialogOpen(false);
            props.onDialogConfirm(name);
          }}
          onClose={() => setPlayerNameDialogOpen(false)}
        />
      </Paper>
      {props.wizardState && (
        <Wizard
          state={props.wizardState.step}
          onNext={props.onWizardNext}
          onClose={props.onWizardClose}
        />
      )}
      {modsDialogOpen && (
        <SelectModsDialog
          title="Select Single Player Mods"
          items={props.installedMods}
          initiallyActiveItems={props.activeMods}
          onSubmit={newMods => {
            setModsDialogOpen(false);
            props.onChangeMods(newMods);
          }}
          onCancel={() => setModsDialogOpen(false)}
        />
      )}
      {configDialogOpen && (
        <RweOptionsDialog
          videoModes={props.videoModes || []}
          initialFullscreen={props.rweConfig && props.rweConfig.fullscreen}
          initiallySelectedMode={
            props.rweConfig && props.rweConfig.width && props.rweConfig.height
              ? {
                  width: props.rweConfig.width,
                  height: props.rweConfig.height,
                }
              : undefined
          }
          initiallySelectedInterfaceMode={
            props.rweConfig && props.rweConfig.interfaceMode
          }
          onSubmit={newConfig => {
            setConfigDialogOpen(false);
            props.onChangeConfig(newConfig);
          }}
          onCancel={() => setConfigDialogOpen(false)}
        />
      )}
    </>
  );
}

function mapStateToProps(state: State): OverviewScreenStateProps {
  return {
    connected: state.masterClient.masterServerConnectionStatus === "connected",
    wizardState: state.wizard,
    installedMods: (state.installedMods || []).map(x => x.name),
    activeMods: state.activeMods,
    hostEnabled: canHostGame(state),
    joinEnabled: canJoinSelectedGame(state),
    launchEnabled: canLaunchRwe(state),
    videoModes: state.videoModes,
    rweConfig: state.rweConfig,
  };
}

function mapDispatchToProps(dispatch: Dispatch): OverviewScreenDispatchProps {
  return {
    onDialogConfirm: (name: string) => {
      dispatch(joinSelectedGameConfirm(name));
    },
    onWizardNext: (path?: string) => {
      dispatch(next(path));
    },
    onWizardClose: () => {
      dispatch(close());
    },
    onChangeMods: (mods: string[]) => {
      dispatch(changeSinglePlayerMods(mods));
    },
    onChangeConfig: (newConfig: RweConfig) => {
      dispatch(submitSettingsDialog(newConfig));
    },
    onHostGame: () => dispatch(hostGame()),
    onLaunchRwe: () => dispatch(launchRwe()),
  };
}

export default connect(mapStateToProps, mapDispatchToProps)(OverviewScreen);
