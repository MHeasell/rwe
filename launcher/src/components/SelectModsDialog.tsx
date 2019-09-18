import * as React from "react";
import {
  ListItem,
  List,
  ListItemText,
  Dialog,
  DialogContent,
  DialogActions,
  Button,
  DialogTitle,
  Checkbox,
  Grid,
  ListItemSecondaryAction,
  Divider,
  IconButton,
} from "@material-ui/core";
import ArrowUpwardIcon from "@material-ui/icons/ArrowUpward";
import ArrowDownwardIcon from "@material-ui/icons/ArrowDownward";

export interface Item {
  name: string;
  checked: boolean;
  selected: boolean;
}

function ModsList(props: {
  items: Item[];
  onSelectMod: (name: string) => void;
  onToggleMod: (name: string) => void;
}) {
  return (
    <List dense>
      {props.items.map(item => {
        return (
          <ListItem
            key={item.name}
            dense
            button
            selected={item.selected}
            onClick={() => {
              props.onSelectMod(item.name);
            }}
          >
            <ListItemText primary={item.name} />
            <ListItemSecondaryAction>
              <Checkbox
                checked={item.checked}
                onChange={() => {
                  props.onToggleMod(item.name);
                }}
              />
            </ListItemSecondaryAction>
          </ListItem>
        );
      })}
    </List>
  );
}

interface Props {
  items?: [Item[], Item[]];
  open: boolean;
  onSelectMod: (name: string) => void;
  onToggleMod: (name: string) => void;
  onModUp: () => void;
  onModDown: () => void;
  onOk: () => void;
  onCancel: () => void;
}

export class SelectModsDialog extends React.Component<Props> {
  render() {
    return (
      <Dialog open={this.props.open} onClose={this.props.onCancel}>
        <DialogTitle>Select Mods</DialogTitle>
        <DialogContent>
          <Grid container>
            <Grid container direction="column" item xs>
              <Grid item>
                <IconButton onClick={this.props.onModUp} size="small">
                  <ArrowUpwardIcon />
                </IconButton>
              </Grid>
              <Grid item>
                <IconButton onClick={this.props.onModDown} size="small">
                  <ArrowDownwardIcon />
                </IconButton>
              </Grid>
            </Grid>
            <Grid item>
              {this.props.items && (
                <>
                  <ModsList
                    items={this.props.items[0]}
                    onSelectMod={this.props.onSelectMod}
                    onToggleMod={this.props.onToggleMod}
                  />
                  <Divider />
                  <ModsList
                    items={this.props.items[1]}
                    onSelectMod={this.props.onSelectMod}
                    onToggleMod={this.props.onToggleMod}
                  />
                </>
              )}
            </Grid>
          </Grid>
        </DialogContent>
        <DialogActions>
          <Button onClick={this.props.onOk} color="primary">
            OK
          </Button>
          <Button onClick={this.props.onCancel}>Cancel</Button>
        </DialogActions>
      </Dialog>
    );
  }
}
