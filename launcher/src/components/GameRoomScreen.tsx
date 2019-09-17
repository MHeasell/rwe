import {
  Button,
  createStyles,
  Divider,
  FormControl,
  InputLabel,
  MenuItem,
  Select,
  TextField,
  Theme,
  Typography,
  WithStyles,
  withStyles,
  RootRef,
} from "@material-ui/core";
import * as React from "react";
import { connect } from "react-redux";
import { Dispatch } from "redux";
import {
  changeColor,
  changeMap,
  changeSide,
  changeTeam,
  closeSelectMapDialog,
  closeSlot,
  dialogSelectMap,
  leaveGame,
  openSelectMapDialog,
  openSlot,
  sendChatMessage,
  sendStartGame,
  toggleReady,
} from "../actions";
import {
  canStartGame,
  ChatMessage,
  getRoom,
  PlayerSide,
  PlayerSlot,
  State,
  SelectedMapDetails,
} from "../state";
import MapSelectDialog from "./MapSelectDialog";
import MessageInput from "./MessageInput";
import { PlayersTable } from "./PlayersTable";

function GameSettingsPanel() {
  return (
    <React.Fragment>
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
    </React.Fragment>
  );
}

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
  mapDialogMapInfo?: SelectedMapDetails;
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

const styles = (theme: Theme) => createStyles({});

interface GameRoomScreenProps
  extends GameRoomScreenStateProps,
    GameRoomScreenDispatchProps,
    WithStyles<typeof styles> {}

class UnconnectedGameRoomScreen extends React.Component<GameRoomScreenProps> {
  private chatDivRef = React.createRef<HTMLElement>();
  private chatBottomRef = React.createRef<HTMLElement>();

  constructor(props: GameRoomScreenProps) {
    super(props);
  }

  getSnapshotBeforeUpdate(prevProps: GameRoomScreenProps) {
    if (prevProps.messages !== this.props.messages) {
      return this.isScrolledToBottom(this.chatDivRef.current!);
    }
    return null;
  }

  componentDidUpdate(
    prevProps: GameRoomScreenProps,
    _prevState: {},
    snapshot: boolean | null
  ) {
    if (snapshot) {
      this.scrollToBottomOfChat();
    }
  }

  render() {
    const messageElements = this.props.messages.map((m, i) => {
      return (
        <Typography key={i}>
          {m.senderName ? m.senderName : "<unknown>"}: {m.message}
        </Typography>
      );
    });

    return (
      <div className="game-room-screen-container">
        <div className="game-room-screen-left">
          <div className="game-room-players-panel">
            <Typography variant="h6" className="game-room-players-title">
              Players
            </Typography>
            <PlayersTable
              rows={this.props.players}
              localPlayerId={this.props.localPlayerId}
              adminPlayerId={this.props.adminPlayerId}
              onOpenSlot={this.props.onOpenSlot}
              onCloseSlot={this.props.onCloseSlot}
              onChangeSide={this.props.onChangeSide}
              onChangeColor={this.props.onChangeColor}
              onChangeTeam={this.props.onChangeTeam}
              onToggleReady={this.props.onToggleReady}
            />
          </div>
          <div className="game-room-map-panel">
            <TextField
              disabled
              value={this.props.mapName ? this.props.mapName : ""}
            />
            <Button
              onClick={this.props.onOpenSelectMapDialog}
              disabled={this.props.localPlayerId !== this.props.adminPlayerId}
            >
              Select Map
            </Button>
          </div>
          <Typography variant="h6" className="game-room-screen-messages-title">
            Messages
          </Typography>
          <RootRef rootRef={this.chatDivRef}>
            <div className="game-room-screen-messages-panel">
              {messageElements}
              <RootRef rootRef={this.chatBottomRef}>
                <div></div>
              </RootRef>
            </div>
          </RootRef>
          <Divider />
          <MessageInput onSend={this.props.onSend} />
        </div>
        <div className="game-room-screen-right">
          <GameSettingsPanel />
          <div>
            <Button fullWidth onClick={this.props.onLeaveGame}>
              Leave Game
            </Button>
          </div>
          <div>
            <Button
              fullWidth
              variant="contained"
              color="primary"
              disabled={!this.props.startEnabled}
              onClick={this.props.onStartGame}
            >
              Start Game
            </Button>
          </div>
        </div>
        <MapSelectDialog
          open={this.props.mapDialogOpen}
          maps={this.props.mapDialogMaps}
          selectedMap={this.props.selectedMap}
          minimapSrc={this.props.mapDialogMinimapPath}
          selectedMapDetails={this.props.mapDialogMapInfo}
          onSelect={this.props.onDialogSelectMap}
          onConfirm={this.props.onChangeMap}
          onClose={this.props.onCloseSelectMapDialog}
        />
      </div>
    );
  }

  private scrollToBottomOfChat() {
    this.chatBottomRef.current!.scrollIntoView();
  }

  private isScrolledToBottom(elem: HTMLElement): boolean {
    const scrollPos = elem.scrollTop;
    const scrollBottom = elem.scrollHeight - elem.clientHeight;
    return scrollBottom <= 0 || scrollPos === scrollBottom;
  }
}

const emptyProps: GameRoomScreenStateProps = {
  players: [],
  messages: [],
  startEnabled: false,
  mapDialogOpen: false,
};

function mapStateToProps(state: State): GameRoomScreenStateProps {
  const room = getRoom(state);
  if (!room) {
    return emptyProps;
  }

  const mapDialog = room.mapDialog;

  return {
    localPlayerId: room.localPlayerId,
    adminPlayerId: room.adminPlayerId,
    players: room.players,
    messages: room.messages,
    startEnabled: canStartGame(room),
    mapName: room.mapName,
    mapDialogOpen: !!mapDialog,
    mapDialogMaps: mapDialog ? mapDialog.maps : undefined,
    selectedMap:
      mapDialog && mapDialog.selectedMap
        ? mapDialog.selectedMap.name
        : undefined,
    mapDialogMinimapPath:
      mapDialog && mapDialog.selectedMap
        ? mapDialog.selectedMap.minimap
        : undefined,
    mapDialogMapInfo:
      mapDialog && mapDialog.selectedMap
        ? mapDialog.selectedMap.details
        : undefined,
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

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(withStyles(styles)(UnconnectedGameRoomScreen));
