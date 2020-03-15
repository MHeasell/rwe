import {
  TableBody,
  TableCell,
  TableHead,
  TableRow,
  Typography,
} from "@material-ui/core";
import {
  createStyles,
  Theme,
  withStyles,
  WithStyles,
} from "@material-ui/core/styles";
import Table from "@material-ui/core/Table";
import * as React from "react";
import { connect } from "react-redux";
import { Dispatch } from "redux";
import { selectGame } from "../actions";
import { State } from "../state";

interface GameTableEntry {
  id: number;
  description: string;
  players: number;
  maxPlayers: number;
}

interface GamesTableStateProps {
  games: GameTableEntry[];
  selectedIndex?: number;
}

interface GamesTableDispatchProps {
  onRowClick?: (id: number) => void;
}

const gamesTableStyles = (theme: Theme) =>
  createStyles({
    head: {
      backgroundColor: "#fafafa",
      position: "sticky",
      top: 0,
    },
  });

interface UnstyledGamesTableProps
  extends GamesTableStateProps,
    GamesTableDispatchProps {}
interface GamesTableProps
  extends UnstyledGamesTableProps,
    WithStyles<typeof gamesTableStyles> {}

const UnstyledGamesTable = (props: GamesTableProps) => {
  const rows = props.games.map((g, i) => {
    const onClick = () => {
      if (props.onRowClick) {
        props.onRowClick(g.id);
      }
    };
    return (
      <TableRow
        key={g.id}
        selected={props.selectedIndex === i}
        onClick={onClick}
      >
        <TableCell>{g.description}</TableCell>
        <TableCell>
          {g.players} / {g.maxPlayers}
        </TableCell>
      </TableRow>
    );
  });

  const rowsOrMessage =
    props.games.length > 0 ? (
      rows
    ) : (
      <TableRow>
        <TableCell colSpan={2}>
          <Typography align="center">
            There are no online games being hosted right now.
          </Typography>
        </TableCell>
      </TableRow>
    );

  return (
    <Table className="games-table">
      <TableHead>
        <TableRow>
          <TableCell className={props.classes.head}>Description</TableCell>
          <TableCell className={props.classes.head}>Players</TableCell>
        </TableRow>
      </TableHead>
      <TableBody>{rowsOrMessage}</TableBody>
    </Table>
  );
};

const UnconnectedGamesTable = withStyles(gamesTableStyles)(UnstyledGamesTable);

function mapStateToProps(state: State): UnstyledGamesTableProps {
  const gameIndex = state.masterClient.games.findIndex(
    g => g.id === state.selectedGameId
  );
  const selectedIndex = gameIndex === -1 ? undefined : gameIndex;
  return {
    games: state.masterClient.games,
    selectedIndex,
  };
}

function mapDispatchToProps(dispatch: Dispatch): GamesTableDispatchProps {
  return {
    onRowClick: (id: number) => dispatch(selectGame(id)),
  };
}

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(UnconnectedGamesTable);
