import { createStyles, Theme, WithStyles, withStyles } from "@material-ui/core";
import Button from "@material-ui/core/Button";
import TextField from "@material-ui/core/TextField";
import * as React from "react";
import { connect } from "react-redux";
import { Dispatch } from "redux";
import { hostGameFormCancel, hostGameFormConfirm } from "../actions";

interface HostGameFormDispatchProps {
  onConfirm: (playerName: string, gameDescription: string, players: number) => void;
  onCancel: () => void;
}

const styles = (theme: Theme) => createStyles({
  cancelButton: {
    "margin-left": theme.spacing.unit,
  },
  nameInput: {
    "flex-grow": 1,
  },
  descriptionInput: {
    "margin-top": theme.spacing.unit,
    "flex-grow": 1,
  },
  playersInput: {
    "margin-top": theme.spacing.unit,
    "flex-grow": 0,
  },
});

interface HostGameFormProps extends HostGameFormDispatchProps, WithStyles<typeof styles> {
}

interface HostGameFormState {
  playerName: string;
  gameDescription: string;
  players: number;
}

class UnconnectedHostGameForm extends React.Component<HostGameFormProps, HostGameFormState> {
  constructor(props: HostGameFormProps) {
    super(props);
    this.state = { playerName: "", gameDescription: "", players: 2 }

    this.handlePlayerNameChange = this.handlePlayerNameChange.bind(this);
    this.handleGameDescriptionChange = this.handleGameDescriptionChange.bind(this);
    this.handlePlayersChange = this.handlePlayersChange.bind(this);
    this.handleSubmit = this.handleSubmit.bind(this);
  }
  handlePlayerNameChange(event: React.SyntheticEvent<EventTarget>) {
    const value = (event.target as HTMLInputElement).value;
    this.setState({ ...this.state, playerName: value });
  }
  handleGameDescriptionChange(event: React.SyntheticEvent<EventTarget>) {
    const value = (event.target as HTMLInputElement).value;
    this.setState({ ...this.state, gameDescription: value });
  }
  handlePlayersChange(event: React.SyntheticEvent<EventTarget>) {
    const value = parseInt((event.target as HTMLSelectElement).value);
    this.setState({ ...this.state, players: value });
  }
  handleSubmit(event: React.SyntheticEvent<EventTarget>) {
    event.preventDefault();
    this.props.onConfirm(this.state.playerName, this.state.gameDescription, this.state.players);
  }

  render() {
    const playerCountList = [2, 3, 4, 5, 6, 7, 8, 9, 10];
    return (
      <div className="host-game-form-container">
        <form onSubmit={this.handleSubmit}>
          <div className="host-game-form-main-panel">
            <TextField className={this.props.classes.nameInput} label="Your Name" value={this.state.playerName} onChange={this.handlePlayerNameChange} />
            <TextField className={this.props.classes.descriptionInput} label="Game Description" value={this.state.gameDescription} onChange={this.handleGameDescriptionChange} />
            <TextField className={this.props.classes.playersInput} select label="Players" value={this.state.players} onChange={this.handlePlayersChange}>
              {playerCountList.map(i => <option key={i} value={i}>{i}</option>)}
            </TextField>
          </div>

          <div className="host-game-form-bottom-panel">
            <Button variant="contained" color="primary" onClick={this.handleSubmit}>Create Game Room</Button>
            <Button type="submit" variant="contained" className={this.props.classes.cancelButton} onClick={this.props.onCancel}>Cancel</Button>
          </div>
        </form>
      </div>
    );
  }
}

function mapDispatchToProps(dispatch: Dispatch): HostGameFormDispatchProps {
  return {
    onConfirm: (playerName: string, gameDescription: string, players: number) => dispatch(hostGameFormConfirm(playerName, gameDescription, players)),
    onCancel: () => dispatch(hostGameFormCancel()),
  };
}

export default connect(undefined, mapDispatchToProps)(withStyles(styles)(UnconnectedHostGameForm));
