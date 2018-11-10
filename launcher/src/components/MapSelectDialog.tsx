import { Button, Dialog, DialogActions, DialogContent, DialogTitle, List, ListItem, ListItemText, Typography, createStyles, Theme, WithStyles, withStyles } from "@material-ui/core";
import * as React from "react";

const styles = (theme: Theme) => createStyles({

});

export interface MapSelectDialogProps extends WithStyles<typeof styles> {
  open: boolean;

  maps?: string[];
  selectedMap?: string;
  minimapSrc?: string;

  onSelect: (map: string) => void;
  onConfirm: () => void;
  onClose: () => void;
}

class MapSelectDialog extends React.Component<MapSelectDialogProps> {

  render() {
    const mapItems = this.props.maps
      ? this.toMapDialogItems(this.props.maps, this.props.selectedMap)
      : <Typography>Maps not yet loaded</Typography>;

    const mapImage = this.props.minimapSrc
      ? <img src={this.props.minimapSrc} className="map-dialog-minimap" />
      : <React.Fragment />;

    return (
      <Dialog open={this.props.open} onClose={this.props.onClose} fullWidth>
        <DialogTitle>Select Map</DialogTitle>
        <DialogContent>
          <div className="map-dialog-container">
            <div className="map-dialog-left">
              <div className="map-dialog-list">
                <List dense>
                  {mapItems}
                </List>
              </div>
            </div>
            <div className="map-dialog-right">
              <div className="map-dialog-minimap-container">
                {mapImage}
              </div>
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

export default withStyles(styles)(MapSelectDialog);
