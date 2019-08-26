import { Button, createStyles, Dialog, DialogActions, DialogContent, DialogTitle, ListItem, ListItemText, RootRef, Theme, Typography, WithStyles, withStyles } from "@material-ui/core";
import * as React from "react";
import { FixedSizeList as List } from "react-window";
import { SelectedMapDetails } from "../state";

const styles = (theme: Theme) => createStyles({

});

export interface MapSelectDialogProps extends WithStyles<typeof styles> {
  open: boolean;

  maps?: string[];
  selectedMap?: string;
  minimapSrc?: string;
  selectedMapDetails?: SelectedMapDetails;

  onSelect: (map: string) => void;
  onConfirm: () => void;
  onClose: () => void;
}

class MapSelectDialog extends React.Component<MapSelectDialogProps> {
  private readonly listRef: React.RefObject<HTMLElement>;
  private readonly selectedElementRef: React.RefObject<HTMLElement>;

  constructor(props: MapSelectDialogProps) {
    super(props);
    this.listRef = React.createRef();
    this.selectedElementRef = React.createRef();
  }

  componentDidMount() {
    this.scrollToSelectedItem();
  }

  componentDidUpdate(prevProps: MapSelectDialogProps) {
    if (!prevProps.maps && this.props.maps) {
      this.scrollToSelectedItem();
    }
  }

  render() {
    const mapImage = this.props.minimapSrc
      ? <img src={`file://${this.props.minimapSrc}`} className="map-dialog-minimap" />
      : <React.Fragment />;

      const mapDescription = this.props.selectedMapDetails
      ? this.props.selectedMapDetails.description : "";
      const mapDescription2 = this.props.selectedMapDetails
      ? `${this.props.selectedMapDetails.memory} Players: ${this.props.selectedMapDetails.numberOfPlayers}` : "";

    return (
      <Dialog open={this.props.open} onClose={this.props.onClose} fullWidth>
        <DialogTitle>Select Map</DialogTitle>
        <DialogContent>
          <div className="map-dialog-container">
            <div className="map-dialog-left">
              <RootRef rootRef={this.listRef}>
                <div className="map-dialog-list">
                  {this.renderMaps()}
                </div>
              </RootRef>
            </div>
            <div className="map-dialog-right">
              <div className="map-dialog-minimap-container">
                {mapImage}
              </div>
              <div className="map-dialog-map-info">
                <Typography>{mapDescription}</Typography>
                <Typography>{mapDescription2}</Typography>
              </div>
            </div>
          </div>
        </DialogContent>
        <DialogActions>
          <Button color="primary" onClick={this.props.onConfirm}>Select Map</Button>
          <Button onClick={this.props.onClose}>Back</Button>
        </DialogActions>
      </Dialog>
    );
  }

  private renderMaps() {
    if (!this.props.maps) {
      return <Typography>Maps not yet loaded</Typography>;
    }

    const Row = ({ index, style }: { index: number, style: React.CSSProperties }) => this.toMapDialogItem(this.props.maps![index], this.props.selectedMap, style);

    return (
      <List height={320} width={292} itemSize={35} itemCount={this.props.maps.length}>
        {Row}
      </List>
    );
  }

  private toMapDialogItem(name: string, selectedMap: string | undefined, style: any) {
    if (name === selectedMap) {
      return (
        <RootRef rootRef={this.selectedElementRef} key={name}>
          <ListItem dense style={style} button selected={name === selectedMap} onClick={() => this.props.onSelect(name)}>
            <ListItemText primary={name}></ListItemText>
          </ListItem>
        </RootRef>
      );
    }
    return (
      <ListItem dense style={style} button key={name} selected={name === selectedMap} onClick={() => this.props.onSelect(name)}>
        <ListItemText primary={name}></ListItemText>
      </ListItem>
    );
  }

  private scrollToSelectedItem() {
    if (this.selectedElementRef.current && this.listRef.current) {
      const elemHeight = this.selectedElementRef.current.offsetHeight;
      const viewportHeight = this.listRef.current.clientHeight;
      const offset = (viewportHeight / 2) - (elemHeight / 2);
      this.listRef.current.scrollTop = this.selectedElementRef.current.offsetTop - offset;
    }
  }
}

export default withStyles(styles)(MapSelectDialog);
