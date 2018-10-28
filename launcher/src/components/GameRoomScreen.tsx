import * as React from "react";
import { State, PlayerInfo, ChatMessage, canStartGame, PlayerSide, PlayerSlot } from "../state";
import { connect } from "react-redux";
import { TextField, WithStyles, createStyles, Theme, withStyles, Button, Table, TableHead, TableRow, TableCell, TableBody, Checkbox, Typography, Divider, Paper, Select, MenuItem, FormControl, InputLabel, Dialog, List, ListItemText, ListItem, DialogContent, DialogActions, DialogTitle } from "@material-ui/core";
import { Dispatch } from "redux";
import { sendChatMessage, leaveGame, toggleReady, sendStartGame, changeSide, changeColor, changeTeam, openSlot, closeSlot, openSelectMapDialog, closeSelectMapDialog, dialogSelectMap, changeMap } from "../actions";
import MessageInput from "./MessageInput";
import { PlayersTable } from "./PlayersTable";

interface GameRoomScreenStateProps {
  localPlayerId?: number;
  adminPlayerId?: number;
  players: PlayerSlot[];
  messages: ChatMessage[];
  startEnabled: boolean;
  mapName?: string;
  mapDialogOpen: boolean;
  mapDialogMaps?: string[];
  mapDialogMinimapPath?: string;
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

const styles = (theme: Theme) => createStyles({
});

interface GameRoomScreenProps extends GameRoomScreenStateProps, GameRoomScreenDispatchProps, WithStyles<typeof styles> {
}

class UnconnectedGameRoomScreen extends React.Component<GameRoomScreenProps> {
  constructor(props: GameRoomScreenProps) {
    super(props);
  }

  render() {
    const messageElements = this.props.messages.map((m, i) => {
      return (
        <Typography key={i}>
          {m.senderName ? m.senderName : "<unknown>"}: {m.message}
        </Typography>
      );
    });

    const mapDialogMaps = this.props.mapDialogMaps
      ? this.toMapDialogItems(this.props.mapDialogMaps, this.props.selectedMap)
      : <Typography>Maps not yet loaded</Typography>;

    const mapImage = this.props.mapDialogMinimapPath
      ? <img width="252" height="252" src={this.props.mapDialogMinimapPath} />
      : <img width="252" height="252" />;

    return (
      <div className="game-room-screen-container">
        <div className="game-room-screen-left">
          <div className="game-room-players-panel">
            <Typography variant="h6" className="game-room-players-title">Players</Typography>
            <PlayersTable
              rows={this.props.players}
              localPlayerId={this.props.localPlayerId}
              adminPlayerId={this.props.adminPlayerId}
              onOpenSlot={this.props.onOpenSlot}
              onCloseSlot={this.props.onCloseSlot}
              onChangeSide={this.props.onChangeSide}
              onChangeColor={this.props.onChangeColor}
              onChangeTeam={this.props.onChangeTeam}
              onToggleReady={this.props.onToggleReady} />
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
          <MessageInput onSend={this.props.onSend} />
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
                {mapImage}
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
    selectedMap: mapDialog && mapDialog.selectedMap ? mapDialog.selectedMap.name : undefined,
    mapDialogMinimapPath: mapDialog && mapDialog.selectedMap ? mapDialog.selectedMap.minimap : undefined,
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
