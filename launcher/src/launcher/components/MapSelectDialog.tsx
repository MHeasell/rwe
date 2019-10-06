import {
  Button,
  createStyles,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  ListItem,
  ListItemText,
  Theme,
  Typography,
  WithStyles,
  withStyles,
} from "@material-ui/core";
import * as React from "react";
import { FixedSizeList as List } from "react-window";

const styles = (theme: Theme) => createStyles({});

export interface SelectedMapDetails {
  description: string;
  memory: string;
  numberOfPlayers: string;
  minimap?: string;
}

export interface MapSelectDialogProps extends WithStyles<typeof styles> {
  open: boolean;

  maps?: string[];
  selectedMap?: string;
  selectedMapDetails?: SelectedMapDetails;

  onSelect: (map: string) => void;
  onConfirm: () => void;
  onClose: () => void;
}

function MapSelectDialog(props: MapSelectDialogProps) {
  const listRef = React.useRef<List>(null);

  React.useEffect(() => {
    if (!props.maps || !props.selectedMap || !listRef.current) {
      return;
    }
    const idx = props.maps!.indexOf(props.selectedMap);
    if (idx === -1) {
      return;
    }
    listRef.current.scrollToItem(idx, "center");
  }, [!!props.maps]);

  const mapImage =
    props.selectedMapDetails && props.selectedMapDetails.minimap ? (
      <img
        src={`file://${props.selectedMapDetails.minimap}`}
        className="map-dialog-minimap"
      />
    ) : (
      <React.Fragment />
    );

  const mapDescription = props.selectedMapDetails
    ? props.selectedMapDetails.description
    : "";
  const mapDescription2 = props.selectedMapDetails
    ? `${props.selectedMapDetails.memory} Players: ${props.selectedMapDetails.numberOfPlayers}`
    : "";

  return (
    <Dialog open={props.open} onClose={props.onClose} fullWidth>
      <DialogTitle>Select Map</DialogTitle>
      <DialogContent>
        <div className="map-dialog-container">
          <div className="map-dialog-left">
            <div className="map-dialog-list">
              <MapsList
                maps={props.maps}
                selectedMap={props.selectedMap}
                onSelect={props.onSelect}
                listRef={listRef}
              />
            </div>
          </div>
          <div className="map-dialog-right">
            <div className="map-dialog-minimap-container">{mapImage}</div>
            <div className="map-dialog-map-info">
              <Typography>{mapDescription}</Typography>
              <Typography>{mapDescription2}</Typography>
            </div>
          </div>
        </div>
      </DialogContent>
      <DialogActions>
        <Button color="primary" onClick={props.onConfirm}>
          Select Map
        </Button>
        <Button onClick={props.onClose}>Back</Button>
      </DialogActions>
    </Dialog>
  );
}

function MapsList(props: {
  maps?: string[];
  selectedMap?: string;
  onSelect: (name: string) => void;
  listRef: React.RefObject<List>;
}) {
  if (!props.maps) {
    return <Typography>Maps not yet loaded</Typography>;
  }

  const Row = ({
    index,
    style,
  }: {
    index: number;
    style: React.CSSProperties;
  }) => {
    const name = props.maps![index];
    return (
      <ListItem
        key={name}
        dense
        style={style}
        button
        selected={name === props.selectedMap}
        onClick={() => props.onSelect(name)}
      >
        <ListItemText primary={name}></ListItemText>
      </ListItem>
    );
  };

  return (
    <List
      ref={props.listRef}
      height={320}
      width={292}
      itemSize={35}
      itemCount={props.maps.length}
    >
      {Row}
    </List>
  );
}

export default withStyles(styles)(MapSelectDialog);
