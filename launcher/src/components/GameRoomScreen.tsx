import * as React from "react";
import { State, PlayerInfo, ChatMessage, canStartGame, PlayerSide, PlayerSlot } from "../state";
import { connect } from "react-redux";
import { TextField, WithStyles, createStyles, Theme, withStyles, Button, Table, TableHead, TableRow, TableCell, TableBody, Checkbox, Typography, Divider, Paper, Select, MenuItem } from "@material-ui/core";
import StarIcon from "@material-ui/icons/Grade";
import { Dispatch } from "redux";
import { sendChatMessage, leaveGame, toggleReady, sendStartGame, changeSide, changeColor, changeTeam } from "../actions";

interface GameRoomScreenStateProps {
  localPlayerId?: number;
  adminPlayerId?: number;
  players: PlayerSlot[];
  messages: ChatMessage[];
  startEnabled: boolean;
}

interface GameRoomScreenDispatchProps {
  onSend: (message: string) => void;
  onLeaveGame: () => void;
  onChangeSide: (side: PlayerSide) => void;
  onChangeTeam: (team?: number) => void;
  onChangeColor: (color: number) => void;
  onToggleReady: () => void;
  onStartGame: () => void;
}

const teamColors = [
  "#1747e7",
  "#d32b00",
  "#fbfbfb",
  "#1b9f13",
  "#071f7b",
  "#7f579f",
  "#ffff00",
  "#2b2b2b",
  "#9bcbdf",
  "#abab83",
];

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
    this.handleSideChange = this.handleSideChange.bind(this);
    this.handleColorChange = this.handleColorChange.bind(this);
    this.handleTeamChange = this.handleTeamChange.bind(this);
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

  handleSideChange(event: React.SyntheticEvent<EventTarget>) {
    this.props.onChangeSide((event.target as HTMLSelectElement).value as PlayerSide);
  }

  handleColorChange(event: React.SyntheticEvent<EventTarget>) {
    this.props.onChangeColor(parseInt((event.target as HTMLSelectElement).value));
  }

  handleTeamChange(event: React.SyntheticEvent<EventTarget>) {
    const value = (event.target as HTMLSelectElement).value;
    const parsedValue = value === "" ? undefined : parseInt(value);
    this.props.onChangeTeam(parsedValue);
  }

  emptyToRow(id: number) {
    return <TableRow key={id}><TableCell colSpan={5}><Typography>Empty Slot</Typography></TableCell></TableRow>;
  }

  playerToRow(id: number, player: PlayerInfo) {
    const checkbox = player.id === this.props.localPlayerId
      ? <Checkbox checked={player.ready} onChange={this.handleReadyChange} />
      : <Checkbox checked={player.ready} disabled />;
    // const nameCell = <TableCell>{player.name}</TableCell>;
    const nameCell = player.id === this.props.adminPlayerId
      ? <TableCell><StarIcon /> {player.name}</TableCell>
      : <TableCell>{player.name}</TableCell>;
    const sideItems = ["ARM", "CORE"].map((x, i) => <MenuItem key={i} value={x}>{x}</MenuItem>);
    const sideSelect = player.id === this.props.localPlayerId
      ? <Select value={player.side} onChange={this.handleSideChange}>{sideItems}</Select>
      : <Select value={player.side} disabled>{sideItems}</Select>;

    const teamItems = ["", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"].map((x, i) => <MenuItem key={i} value={x}>{x}</MenuItem>);
    const playerTeamValue = player.team === undefined ? "" : player.team.toString();
    const teamSelect = player.id === this.props.localPlayerId
      ? <Select value={playerTeamValue} onChange={this.handleTeamChange}>{teamItems}</Select>
      : <Select value={playerTeamValue} onChange={this.handleTeamChange} disabled>{teamItems}</Select>;

    const colorItems = teamColors.map((c, i) => {
      const style = {
        width: "16px",
        height: "16px",
        backgroundColor: c,
      };
      return <MenuItem key={i} value={i}><div style={style}></div></MenuItem>;
    });
    const colorSelect = player.id === this.props.localPlayerId
      ? <Select value={player.color} onChange={this.handleColorChange}>{colorItems}</Select>
      : <Select value={player.color} disabled>{colorItems}</Select>;

    return (
      <TableRow key={id}>
        {nameCell}
        <TableCell>
          {sideSelect}
        </TableCell>
        <TableCell>
          {colorSelect}
        </TableCell>
        <TableCell>
          {teamSelect}
        </TableCell>
        <TableCell padding="checkbox">{checkbox}</TableCell>
      </TableRow>
    );
  }

  render() {
    const messageElements = this.props.messages.map((m, i) => {
      return (
        <Typography key={i}>
          {m.senderName ? m.senderName : "<unknown>"}: {m.message}
        </Typography>
      );
    });
    const rows = this.props.players.map((slot, i) => {
      switch (slot.state) {
        case "empty": return this.emptyToRow(i);
        case "filled": return this.playerToRow(i, slot.player);
        default: throw new Error("Unknown slot state");
      }
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
    onChangeColor: (color: number) => dispatch(changeColor(color)),
    onChangeSide: (side: PlayerSide) => dispatch(changeSide(side)),
    onChangeTeam: (team?: number) => dispatch(changeTeam(team)),
    onToggleReady: () => dispatch(toggleReady()),
    onStartGame: () => dispatch(sendStartGame()),
  };
}

export default connect(mapStateToProps, mapDispatchToProps)(withStyles(styles)(UnconnectedGameRoomScreen));
