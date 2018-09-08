import * as React from "react";
import { State, PlayerInfo, ChatMessage, canStartGame } from "../state";
import { connect } from "react-redux";
import { TextField, WithStyles, createStyles, Theme, withStyles, Button, Table, TableHead, TableRow, TableCell, TableBody, Checkbox, Typography, Divider, Paper } from "@material-ui/core";
import StarIcon from "@material-ui/icons/Grade";
import { Dispatch } from "redux";
import { sendChatMessage, leaveGame, toggleReady, sendStartGame } from "../actions";

interface GameRoomScreenStateProps {
  localPlayerId?: number;
  adminPlayerId?: number;
  players: PlayerInfo[];
  messages: ChatMessage[];
  startEnabled: boolean;
}

interface GameRoomScreenDispatchProps {
  onSend: (message: string) => void;
  onLeaveGame: () => void;
  onToggleReady: () => void;
  onStartGame: () => void;
}

const styles = (theme: Theme) => createStyles({
  messageInput: {
    "flex-grow": 1,
  },
});

interface GameRoomScreenProps extends GameRoomScreenStateProps, GameRoomScreenDispatchProps, WithStyles<typeof styles> {
}

interface GameRoomScreenState {
  value: string;
}

class UnconnectedGameRoomScreen extends React.Component<GameRoomScreenProps, GameRoomScreenState> {
  constructor(props: GameRoomScreenProps) {
    super(props);
    this.state = { value: "" };

    this.handleUserMessageChange = this.handleUserMessageChange.bind(this);
    this.handleSend = this.handleSend.bind(this);
    this.handleReadyChange = this.handleReadyChange.bind(this);
  }

  handleUserMessageChange(event: React.SyntheticEvent<EventTarget>) {
    this.setState({value: (event.target as HTMLInputElement).value});
  }

  handleSend(event: React.SyntheticEvent<EventTarget>) {
    event.preventDefault();
    if (this.state.value) {
      this.setState({value: ""});
      this.props.onSend(this.state.value);
    }
  }

  handleReadyChange(event: React.SyntheticEvent<EventTarget>) {
    this.props.onToggleReady();
  }

  render() {
    const messageElements = this.props.messages.map((m, i) => {
      return (
        <Typography key={i}>
          {m.senderName ? m.senderName : "<unknown>"}: {m.message}
        </Typography>
      );
    });
    const rows = this.props.players.map((player) => {
      const checkbox = player.id === this.props.localPlayerId
        ? <Checkbox checked={player.ready} onChange={this.handleReadyChange} />
        : <Checkbox checked={player.ready} disabled />;
      // const nameCell = <TableCell>{player.name}</TableCell>;
      const nameCell = player.id === this.props.adminPlayerId
        ? <TableCell><StarIcon /> {player.name}</TableCell>
        : <TableCell>{player.name}</TableCell>;
      return (
        <TableRow key={player.id}>
          {nameCell}
          <TableCell>{player.side}</TableCell>
          <TableCell>{player.color}</TableCell>
          <TableCell>{player.team}</TableCell>
          <TableCell padding="checkbox">{checkbox}</TableCell>
        </TableRow>
      );
    });
    return (
      <div className="game-room-screen-container">
        <div className="game-room-screen-left">
          <div className="game-room-players-panel">
            <Typography variant="title" className="game-room-players-title">Players</Typography>
            <Table>
              <TableHead>
                <TableRow>
                  <TableCell>Player</TableCell>
                  <TableCell>Side</TableCell>
                  <TableCell>Color</TableCell>
                  <TableCell>Team</TableCell>
                  <TableCell>Ready?</TableCell>
                </TableRow>
              </TableHead>
              <TableBody>
                {rows}
              </TableBody>
            </Table>
          </div>
          <Typography variant="title" className="game-room-screen-messages-title">Messages</Typography>
          <div className="game-room-screen-messages-panel">
            {messageElements}
          </div>
          <Divider />
          <form className="game-room-screen-bottom-panel" onSubmit={this.handleSend}>
            <TextField className={this.props.classes.messageInput} value={this.state.value} onChange={this.handleUserMessageChange} />
            <Button type="submit">Send</Button>
          </form>
        </div>
        <div className="game-room-screen-right">
          <div>
            <Button onClick={this.props.onLeaveGame}>Leave Game</Button>
          </div>
          <div>
            <Button variant="contained" color="primary" disabled={!this.props.startEnabled} onClick={this.props.onStartGame}>Start Game</Button>
          </div>
        </div>
      </div>
    );
  }
}

function mapStateToProps(state: State): GameRoomScreenStateProps {
  if (!state.currentGame) {
    return {
      players: [],
      messages: [],
      startEnabled: false,
    };
  }

  return {
    localPlayerId: state.currentGame.localPlayerId,
    adminPlayerId: state.currentGame.adminPlayerId,
    players: state.currentGame.players,
    messages: state.currentGame.messages,
    startEnabled: canStartGame(state.currentGame),
  };
}

function mapDispatchToProps(dispatch: Dispatch): GameRoomScreenDispatchProps {
  return {
    onSend: (message: string) => dispatch(sendChatMessage(message)),
    onLeaveGame: () => dispatch(leaveGame()),
    onToggleReady: () => dispatch(toggleReady()),
    onStartGame: () => dispatch(sendStartGame()),
  };
}

export default connect(mapStateToProps, mapDispatchToProps)(withStyles(styles)(UnconnectedGameRoomScreen));
