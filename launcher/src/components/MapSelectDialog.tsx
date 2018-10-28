import { Button, Dialog, DialogActions, DialogContent, DialogTitle, List, ListItem, ListItemText, Typography } from "@material-ui/core";
import * as React from "react";

export interface MapSelectDialogProps {
  open: boolean;

  maps?: string[];
  selectedMap?: string;
  minimapSrc?: string;

  onSelect: (map: string) => void;
  onConfirm: () => void;
  onClose: () => void;
}

export class MapSelectDialog extends React.Component<MapSelectDialogProps> {

  render() {
    const mapItems = this.props.maps
      ? this.toMapDialogItems(this.props.maps, this.props.selectedMap)
      : <Typography>Maps not yet loaded</Typography>;

    const mapImage = this.props.minimapSrc
      ? <img width="252" height="252" src={this.props.minimapSrc} />
      : <img width="252" height="252" />;

    return (
      <Dialog open={this.props.open} onClose={this.props.onClose} fullWidth>
        <DialogTitle>Select Map</DialogTitle>
        <DialogContent>
          <div className="map-dialog-container">
            <div className="map-dialog-left">
              <div className="map-dialog-list">
                <List>
                  {mapItems}
                </List>
              </div>
            </div>
            <div className="map-dialog-right">
              {mapImage}
            </div>
          </div>
        </DialogContent>
        <DialogActions>
          <Button onClick={this.props.onConfirm}>Select Map</Button>
          <Button onClick={this.props.onClose}>Back</Button>
        </DialogActions>
      </Dialog>
    );
  }

  private toMapDialogItems(maps: string[], selectedMap?: string) {
    return maps.map(name => {
      return (
        <ListItem button key={name} selected={name === selectedMap} onClick={() => this.props.onSelect(name)}>
          <ListItemText primary={name}></ListItemText>
        </ListItem>
      );
    });
  }
}
