import * as React from "react";
import { State, PlayerInfo, ChatMessage } from "../state";
import { connect } from "react-redux";
import { TextField, WithStyles, createStyles, Theme, withStyles, Button, Table, TableHead, TableRow, TableCell, TableBody, Checkbox, Typography } from "@material-ui/core";
import { Dispatch } from "redux";
import { sendChatMessage, leaveGame, toggleReady, startGameThunk, sendStartGame } from "../actions";

interface GameRoomScreenStateProps {
  localPlayerId?: number;
  players: PlayerInfo[];
  messages: ChatMessage[];
  userMessage: string;
  canStartGame: boolean;
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
    this.state = {value: ""}

    this.handleUserMessageChange = this.handleUserMessageChange.bind(this);
    this.handleSend = this.handleSend.bind(this);
    this.handleReadyChange = this.handleReadyChange.bind(this);
  }

  handleUserMessageChange(event: React.SyntheticEvent<EventTarget>) {
    this.setState({value: (event.target as HTMLInputElement).value});
  }

  handleSend(event: React.SyntheticEvent<EventTarget>) {
    event.preventDefault();
    this.setState({value: ""});
    this.props.onSend(this.state.value);
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
        : <Checkbox checked={player.ready} disabled />
      return (
        <TableRow key={player.id}>
          <TableCell>{player.name}</TableCell>
          <TableCell>{player.side}</TableCell>
          <TableCell>{player.color}</TableCell>
          <TableCell>{player.team}</TableCell>
          <TableCell>{checkbox}</TableCell>
        </TableRow>
      );
    });
    return (
      <div className="game-room-screen-container">
        <div className="game-room-screen-left">
          <div className="game-room-players-panel">
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
          <div className="game-room-screen-messages-panel">
            {messageElements}
          </div>
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
            <Button variant="contained" color="primary" disabled={!this.props.canStartGame} onClick={this.props.onStartGame}>Start Game</Button>
          </div>
        </div>
      </div>
    );
  };
}

function mapStateToProps(state: State): GameRoomScreenStateProps {
  const localPlayerId = state.currentGame ? state.currentGame.localPlayerId : undefined;
  const players = state.currentGame ? state.currentGame.players : [];
  const messages = state.currentGame ? state.currentGame.messages : [];
  const userMessage = state.currentScreen.screen == "game-room" ? state.currentScreen.userMessage : "";
  const canStartGame = state.currentGame ? state.currentGame.adminPlayerId === state.currentGame.localPlayerId && state.currentGame.players.length > 0 && state.currentGame.players.every(x => x.ready) : false;
  return { localPlayerId, players, messages, userMessage, canStartGame };
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
