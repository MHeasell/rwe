import {
  Checkbox,
  MenuItem,
  Select,
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableRow,
} from "@material-ui/core";
import StarIcon from "@material-ui/icons/Grade";
import * as React from "react";
import { PlayerInfo, PlayerSide, PlayerSlot } from "../gameClient/state";
import { assertNever } from "../../common/util";

type OpenStatus = "open" | "closed";
const openStatuses: OpenStatus[] = ["open", "closed"];
function statusToLabel(status: OpenStatus) {
  switch (status) {
    case "open":
      return "Open Slot";
    case "closed":
      return "Closed Slot";
    default:
      assertNever(status);
  }
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

export interface PlayersTableProps {
  rows: PlayerSlot[];
  localPlayerId?: number;
  adminPlayerId?: number;

  onToggleReady: () => void;
  onChangeSide: (side: PlayerSide) => void;
  onChangeColor: (color: number) => void;
  onChangeTeam: (team: number | undefined) => void;
  onOpenSlot: (slotId: number) => void;
  onCloseSlot: (slotId: number) => void;
}

export class PlayersTable extends React.Component<PlayersTableProps> {
  constructor(props: PlayersTableProps) {
    super(props);

    this.handleReadyChange = this.handleReadyChange.bind(this);
    this.handleSideChange = this.handleSideChange.bind(this);
    this.handleColorChange = this.handleColorChange.bind(this);
    this.handleTeamChange = this.handleTeamChange.bind(this);
  }

  render() {
    const rows: JSX.Element[] = this.props.rows.map((slot, i) => {
      switch (slot.state) {
        case "empty":
          return this.emptyToRow(i, "open");
        case "closed":
          return this.emptyToRow(i, "closed");
        case "filled":
          return this.playerToRow(i, slot.player);
        default:
          return assertNever(slot);
      }
    });

    return (
      <Table size="small">
        <TableHead>
          <TableRow>
            <TableCell>Player</TableCell>
            <TableCell>Side</TableCell>
            <TableCell>Color</TableCell>
            <TableCell>Team</TableCell>
            <TableCell>Ready?</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>{rows}</TableBody>
      </Table>
    );
  }

  private emptyToRow(id: number, openStatus: OpenStatus) {
    const items = openStatuses.map((x, i) => (
      <MenuItem key={i} value={x}>
        {statusToLabel(x)}
      </MenuItem>
    ));
    const select =
      this.props.localPlayerId === this.props.adminPlayerId ? (
        <Select
          value={openStatus}
          onChange={e => this.handleOpenStatusChange(id, e)}
        >
          {items}
        </Select>
      ) : (
        <Select value={openStatus} disabled>
          {items}
        </Select>
      );
    return (
      <TableRow key={id}>
        <TableCell>{select}</TableCell>
        <TableCell colSpan={4}></TableCell>
      </TableRow>
    );
  }

  private playerToRow(id: number, player: PlayerInfo) {
    const checkbox =
      player.id === this.props.localPlayerId ? (
        <Checkbox checked={player.ready} onChange={this.handleReadyChange} />
      ) : (
        <Checkbox checked={player.ready} disabled />
      );
    // const nameCell = <TableCell>{player.name}</TableCell>;
    const nameCell =
      player.id === this.props.adminPlayerId ? (
        <TableCell>
          <StarIcon /> {player.name}
        </TableCell>
      ) : (
        <TableCell>{player.name}</TableCell>
      );
    const sideItems = ["ARM", "CORE"].map((x, i) => (
      <MenuItem key={i} value={x}>
        {x}
      </MenuItem>
    ));
    const sideSelect =
      player.id === this.props.localPlayerId ? (
        <Select value={player.side} onChange={this.handleSideChange}>
          {sideItems}
        </Select>
      ) : (
        <Select value={player.side} disabled>
          {sideItems}
        </Select>
      );

    const teamItems = [
      "",
      "1",
      "2",
      "3",
      "4",
      "5",
      "6",
      "7",
      "8",
      "9",
      "10",
    ].map((x, i) => (
      <MenuItem key={i} value={x}>
        {x}
      </MenuItem>
    ));
    const playerTeamValue =
      player.team === undefined ? "" : player.team.toString();
    const teamSelect =
      player.id === this.props.localPlayerId ? (
        <Select value={playerTeamValue} onChange={this.handleTeamChange}>
          {teamItems}
        </Select>
      ) : (
        <Select
          value={playerTeamValue}
          onChange={this.handleTeamChange}
          disabled
        >
          {teamItems}
        </Select>
      );

    const colorItems = teamColors.map((c, i) => {
      const style = {
        width: "16px",
        height: "16px",
        backgroundColor: c,
      };
      return (
        <MenuItem key={i} value={i}>
          <div style={style}></div>
        </MenuItem>
      );
    });
    const colorSelect =
      player.id === this.props.localPlayerId ? (
        <Select value={player.color} onChange={this.handleColorChange}>
          {colorItems}
        </Select>
      ) : (
        <Select value={player.color} disabled>
          {colorItems}
        </Select>
      );

    return (
      <TableRow key={id}>
        {nameCell}
        <TableCell>{sideSelect}</TableCell>
        <TableCell>{colorSelect}</TableCell>
        <TableCell>{teamSelect}</TableCell>
        <TableCell padding="checkbox">{checkbox}</TableCell>
      </TableRow>
    );
  }

  private handleReadyChange(event: React.SyntheticEvent<EventTarget>) {
    this.props.onToggleReady();
  }

  private handleSideChange(event: React.SyntheticEvent<EventTarget>) {
    this.props.onChangeSide(
      (event.target as HTMLSelectElement).value as PlayerSide
    );
  }

  private handleColorChange(event: React.SyntheticEvent<EventTarget>) {
    this.props.onChangeColor(
      parseInt((event.target as HTMLSelectElement).value)
    );
  }

  private handleTeamChange(event: React.SyntheticEvent<EventTarget>) {
    const value = (event.target as HTMLSelectElement).value;
    const parsedValue = value === "" ? undefined : parseInt(value);
    this.props.onChangeTeam(parsedValue);
  }

  private handleOpenStatusChange(
    slotId: number,
    event: React.SyntheticEvent<EventTarget>
  ) {
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
}
