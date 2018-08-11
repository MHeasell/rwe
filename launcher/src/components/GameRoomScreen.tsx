import * as React from "react";
import { State, PlayerInfo } from "../reducers";
import { connect } from "react-redux";
import { TextField, WithStyles, createStyles, Theme, withStyles, Button, Table, TableHead, TableRow, TableCell, TableBody } from "@material-ui/core";
import { Dispatch } from "redux";
import { sendChatMessage, leaveGame } from "../actions";

interface GameRoomScreenStateProps {
  players: PlayerInfo[];
  messages: string[];
  userMessage: string;
}

interface GameRoomScreenDispatchProps {
  onSend: (message: string) => void;
  onLeaveGame: () => void;
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
  }

  handleUserMessageChange(event: React.SyntheticEvent<EventTarget>) {
    this.setState({value: (event.target as HTMLInputElement).value});
  }

  handleSend(event: React.SyntheticEvent<EventTarget>) {
    event.preventDefault();
    this.setState({value: ""});
    this.props.onSend(this.state.value);
  }

  render() {
    const messageElements = this.props.messages.map((m, i) => <div key={i}>{m}</div>);
    const rows = this.props.players.map((player) => {
      return (
        <TableRow key={player.id}>
          <TableCell>{player.name}</TableCell>
          <TableCell>{player.side}</TableCell>
          <TableCell>{player.color}</TableCell>
          <TableCell>{player.team}</TableCell>
          <TableCell>{player.ready?"Yes":"No"}</TableCell>
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
                  <TableCell>Go?</TableCell>
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
          <Button onClick={this.props.onLeaveGame}>Leave Game</Button>
        </div>
      </div>
    );
  };
}

function mapStateToProps(state: State): GameRoomScreenStateProps {
  const players = state.currentGame ? state.currentGame.players : [];
  const messages = state.currentGame ? state.currentGame.messages : [];
  const userMessage = state.currentScreen.screen == "game-room" ? state.currentScreen.userMessage : "";
  return { players, messages, userMessage };
}

function mapDispatchToProps(dispatch: Dispatch): GameRoomScreenDispatchProps {
  return {
    onSend: (message: string) => dispatch(sendChatMessage(message)), // TODO: replace with actual chat message
    onLeaveGame: () => dispatch(leaveGame()),
  };
}

export default connect(mapStateToProps, mapDispatchToProps)(withStyles(styles)(UnconnectedGameRoomScreen));
