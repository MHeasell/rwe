import * as React from "react";
import { State, PlayerInfo, ChatMessage, canStartGame, PlayerSide, PlayerSlot } from "../state";
import { connect } from "react-redux";
import { TextField, WithStyles, createStyles, Theme, withStyles, Button, Table, TableHead, TableRow, TableCell, TableBody, Checkbox, Typography, Divider, Paper, Select, MenuItem, FormControl, InputLabel, Dialog, List, ListItemText, ListItem, DialogContent, DialogActions, DialogTitle } from "@material-ui/core";
import StarIcon from "@material-ui/icons/Grade";
import { Dispatch } from "redux";
import { sendChatMessage, leaveGame, toggleReady, sendStartGame, changeSide, changeColor, changeTeam, openSlot, closeSlot, openSelectMapDialog, closeSelectMapDialog, dialogSelectMap, changeMap } from "../actions";

interface GameRoomScreenStateProps {
  localPlayerId?: number;
  adminPlayerId?: number;
  players: PlayerSlot[];
  messages: ChatMessage[];
  startEnabled: boolean;
  mapName?: string;
  mapDialogOpen: boolean;
  mapDialogMaps?: string[];
  selectedMap?: string;
}

interface GameRoomScreenDispatchProps {
  onSend: (message: string) => void;
  onLeaveGame: () => void;
  onOpenSlot: (slotId: number) => void;
  onCloseSlot: (slotId: number) => void;
  onChangeSide: (side: PlayerSide) => void;
  onChangeTeam: (team?: number) => void;
  onChangeColor: (color: number) => void;
  onToggleReady: () => void;
  onStartGame: () => void;
  onOpenSelectMapDialog: () => void;
  onCloseSelectMapDialog: () => void;
  onDialogSelectMap: (mapName: string) => void;
  onChangeMap: () => void;
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

type OpenStatus = "open" | "closed";
const openStatuses: OpenStatus[] = ["open", "closed"];
function statusToLabel(status: OpenStatus) {
  switch (status) {
    case "open": return "Open Slot";
    case "closed": return "Closed Slot";
  }
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

  handleOpenStatusChange(slotId: number, event: React.SyntheticEvent<EventTarget>) {
    const value = (event.target as HTMLSelectElement).value as OpenStatus;
    switch (value) {
      case "open":
        this.props.onOpenSlot(slotId);
        return;
      case "closed":
        this.props.onCloseSlot(slotId);
        return;
    }
  }

  emptyToRow(id: number, openStatus: OpenStatus) {
    const items = openStatuses.map((x, i) => <MenuItem key={i} value={x}>{statusToLabel(x)}</MenuItem>);
    const select = this.props.localPlayerId === this.props.adminPlayerId
      ? <Select value={openStatus} onChange={e => this.handleOpenStatusChange(id, e)}>{items}</Select>
      : <Select value={openStatus} disabled>{items}</Select>;
    return (
      <TableRow key={id}>
        <TableCell>{select}</TableCell>
        <TableCell colSpan={4}></TableCell>
      </TableRow>
    );
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
    const rows: JSX.Element[] = this.props.players.map((slot, i) => {
      switch (slot.state) {
        case "empty": return this.emptyToRow(i, "open");
        case "closed": return this.emptyToRow(i, "closed");
        case "filled": return this.playerToRow(i, slot.player);
      }
    });

    const mapDialogMaps = this.props.mapDialogMaps
      ? this.toMapDialogItems(this.props.mapDialogMaps, this.props.selectedMap)
      : <Typography>Maps not yet loaded</Typography>;

    return (
      <div className="game-room-screen-container">
        <div className="game-room-screen-left">
          <div className="game-room-players-panel">
            <Typography variant="h6" className="game-room-players-title">Players</Typography>
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
          <div className="game-room-map-panel">
            <TextField disabled value={this.props.mapName ? this.props.mapName : ""} />
            <Button onClick={this.props.onOpenSelectMapDialog} disabled={this.props.localPlayerId !== this.props.adminPlayerId}>Select Map</Button>
          </div>
          <Typography variant="h6" className="game-room-screen-messages-title">Messages</Typography>
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
            <FormControl fullWidth>
              <InputLabel shrink>Commander Dies</InputLabel>
              <Select value="continues">
                <MenuItem value="continues">Continues</MenuItem>
                <MenuItem value="ends">Game Ends</MenuItem>
              </Select>
            </FormControl>
          </div>
          <div>
            <FormControl fullWidth>
              <InputLabel shrink>Location</InputLabel>
              <Select value="fixed">
                <MenuItem value="fixed">Fixed</MenuItem>
                <MenuItem value="random">Random</MenuItem>
              </Select>
            </FormControl>
          </div>
          <div>
            <FormControl fullWidth>
              <InputLabel shrink>Mapping Mode</InputLabel>
              <Select value="unmapped">
                <MenuItem value="unmapped">Unmapped</MenuItem>
                <MenuItem value="mapped">Mapped</MenuItem>
              </Select>
            </FormControl>
          </div>
          <div>
            <FormControl fullWidth>
              <InputLabel shrink>Line of Sight</InputLabel>
              <Select value="true">
                <MenuItem value="true">True</MenuItem>
                <MenuItem value="circular">Circular</MenuItem>
                <MenuItem value="permanent">Permanent</MenuItem>
              </Select>
            </FormControl>
          </div>
          <div>
            <Button fullWidth onClick={this.props.onLeaveGame}>Leave Game</Button>
          </div>
          <div>
            <Button fullWidth variant="contained" color="primary" disabled={!this.props.startEnabled} onClick={this.props.onStartGame}>Start Game</Button>
          </div>
        </div>
        <Dialog open={this.props.mapDialogOpen} onClose={this.props.onCloseSelectMapDialog} fullWidth>
          <DialogTitle>Select Map</DialogTitle>
          <DialogContent>
            <div className="map-dialog-container">
              <div className="map-dialog-left">
                <div className="map-dialog-list">
                  <List>
                    {mapDialogMaps}
                  </List>
                </div>
              </div>
              <div className="map-dialog-right">
                <img width="252" height="252" />
              </div>
            </div>
          </DialogContent>
          <DialogActions>
            <Button onClick={this.props.onChangeMap}>Select Map</Button>
            <Button onClick={this.props.onCloseSelectMapDialog}>Back</Button>
          </DialogActions>
        </Dialog>
      </div>
    );
  }

  private toMapDialogItems(maps: string[], selectedMap?: string) {
    return maps.map(name => {
      return (
        <ListItem button key={name} selected={name === selectedMap} onClick={() => this.props.onDialogSelectMap(name)}>
          <ListItemText primary={name}></ListItemText>
        </ListItem>
      );
    });
  }
}

function mapStateToProps(state: State): GameRoomScreenStateProps {
  if (!state.currentGame) {
    return {
      players: [],
      messages: [],
      startEnabled: false,
      mapDialogOpen: false,
    };
  }

  const mapDialog = state.currentGame.mapDialog;

  return {
    localPlayerId: state.currentGame.localPlayerId,
    adminPlayerId: state.currentGame.adminPlayerId,
    players: state.currentGame.players,
    messages: state.currentGame.messages,
    startEnabled: canStartGame(state.currentGame),
    mapName: state.currentGame.mapName,
    mapDialogOpen: !!mapDialog,
    mapDialogMaps: mapDialog ? mapDialog.maps : undefined,
    selectedMap: mapDialog ? mapDialog.selectedMap : undefined,
  };
}

function mapDispatchToProps(dispatch: Dispatch): GameRoomScreenDispatchProps {
  return {
    onSend: (message: string) => dispatch(sendChatMessage(message)),
    onLeaveGame: () => dispatch(leaveGame()),
    onOpenSlot: (slotId: number) => dispatch(openSlot(slotId)),
    onCloseSlot: (slotId: number) => dispatch(closeSlot(slotId)),
    onChangeColor: (color: number) => dispatch(changeColor(color)),
    onChangeSide: (side: PlayerSide) => dispatch(changeSide(side)),
    onChangeTeam: (team?: number) => dispatch(changeTeam(team)),
    onToggleReady: () => dispatch(toggleReady()),
    onStartGame: () => dispatch(sendStartGame()),
    onOpenSelectMapDialog: () => dispatch(openSelectMapDialog()),
    onCloseSelectMapDialog: () => dispatch(closeSelectMapDialog()),
    onDialogSelectMap: (mapName: string) => dispatch(dialogSelectMap(mapName)),
    onChangeMap: () => dispatch(changeMap()),
  };
}

export default connect(mapStateToProps, mapDispatchToProps)(withStyles(styles)(UnconnectedGameRoomScreen));
