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
  leaveGame,
  openSelectMapDialog,
  openSlot,
  sendChatMessage,
  sendStartGame,
  toggleReady,
  setActiveMods,
} from "../actions";
import { getRoom, State } from "../state";
import MapSelectDialog, { SelectedMapDetails } from "./MapSelectDialog";
import MessageInput from "./MessageInput";
import { PlayersTable } from "./PlayersTable";
import { SelectModsDialog } from "./SelectModsDialog";
import { dialogSelectMap } from "../mapsDialogActions";
import {
  ChatMessage,
  canStartGame,
  PlayerSlot,
  PlayerSide,
} from "../gameClient/state";

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
  activeMods: string[];
  installedMods: string[];
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
  onChangeMods: (mods: string[]) => void;
}

const styles = (theme: Theme) => createStyles({});

interface GameRoomScreenProps
  extends GameRoomScreenStateProps,
    GameRoomScreenDispatchProps,
    WithStyles<typeof styles> {}

function UnconnectedGameRoomScreen(props: GameRoomScreenProps) {
  const chatDivRef = React.useRef<HTMLElement>();
  const chatBottomRef = React.useRef<HTMLElement>();

  const isScrolledToBottom = React.useMemo(() => {
    return chatDivRef.current
      ? isElementScrolledToBottom(chatDivRef.current)
      : null;
  }, [props.messages]);

  React.useEffect(() => {
    if (isScrolledToBottom) {
      chatBottomRef.current!.scrollIntoView();
    }
  }, [props.messages]);

  const [modsDialogOpen, setModsDialogOpen] = React.useState(false);

  const messageElements = props.messages.map((m, i) => {
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
            rows={props.players}
            localPlayerId={props.localPlayerId}
            adminPlayerId={props.adminPlayerId}
            onOpenSlot={props.onOpenSlot}
            onCloseSlot={props.onCloseSlot}
            onChangeSide={props.onChangeSide}
            onChangeColor={props.onChangeColor}
            onChangeTeam={props.onChangeTeam}
            onToggleReady={props.onToggleReady}
          />
        </div>
        <div className="game-room-map-panel">
          <TextField disabled value={props.mapName ? props.mapName : ""} />
          <Button
            onClick={props.onOpenSelectMapDialog}
            disabled={props.localPlayerId !== props.adminPlayerId}
          >
            Select Map
          </Button>
          <Button onClick={() => setModsDialogOpen(true)}>Select Mods</Button>
        </div>
        <Typography variant="h6" className="game-room-screen-messages-title">
          Messages
        </Typography>
        <RootRef rootRef={chatDivRef}>
          <div className="game-room-screen-messages-panel">
            {messageElements}
            <RootRef rootRef={chatBottomRef}>
              <div></div>
            </RootRef>
          </div>
        </RootRef>
        <Divider />
        <MessageInput onSend={props.onSend} />
      </div>
      <div className="game-room-screen-right">
        <GameSettingsPanel />
        <div>
          <Button fullWidth onClick={props.onLeaveGame}>
            Leave Game
          </Button>
        </div>
        <div>
          <Button
            fullWidth
            variant="contained"
            color="primary"
            disabled={!props.startEnabled}
            onClick={props.onStartGame}
          >
            Start Game
          </Button>
        </div>
      </div>
      <MapSelectDialog
        open={props.mapDialogOpen}
        maps={props.mapDialogMaps}
        selectedMap={props.selectedMap}
        selectedMapDetails={props.mapDialogMapInfo}
        onSelect={props.onDialogSelectMap}
        onConfirm={props.onChangeMap}
        onClose={props.onCloseSelectMapDialog}
      />
      {modsDialogOpen && (
        <SelectModsDialog
          title="Select Mods"
          initiallyActiveItems={props.activeMods}
          items={props.installedMods}
          onSubmit={items => {
            setModsDialogOpen(false);
            props.onChangeMods(items);
          }}
          onCancel={() => setModsDialogOpen(false)}
        />
      )}
    </div>
  );
}

function isElementScrolledToBottom(elem: HTMLElement): boolean {
  const scrollPos = elem.scrollTop;
  const scrollBottom = elem.scrollHeight - elem.clientHeight;
  return scrollBottom <= 0 || scrollPos === scrollBottom;
}

function mapStateToProps(state: State): GameRoomScreenStateProps {
  const installedMods = (state.installedMods || []).map(x => x.name);
  const game = state.currentGame;
  const room = getRoom(state);
  if (!game || !room) {
    return {
      installedMods,
      activeMods: [],
      players: [],
      messages: [],
      startEnabled: false,
      mapDialogOpen: false,
    };
  }

  const mapDialog = room.mapDialog;

  return {
    installedMods,
    activeMods: game.activeMods,
    localPlayerId: game.localPlayerId,
    adminPlayerId: game.adminPlayerId,
    players: game.players,
    messages: game.messages,
    startEnabled: canStartGame(game),
    mapName: game.mapName,
    mapDialogOpen: !!mapDialog,
    mapDialogMaps: mapDialog?.maps?.map(x => x.name),
    selectedMap: mapDialog ? mapDialog.selectedMap : undefined,
    mapDialogMapInfo:
      mapDialog && mapDialog.selectedMap && room.mapCache[mapDialog.selectedMap]
        ? room.mapCache[mapDialog.selectedMap]
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
    onChangeMods: (mods: string[]) => dispatch(setActiveMods(mods)),
  };
}

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(withStyles(styles)(UnconnectedGameRoomScreen));
