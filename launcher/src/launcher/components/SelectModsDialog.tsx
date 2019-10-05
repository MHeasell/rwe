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

export function SelectModsDialog(props: Props) {
  return (
    <Dialog open={props.open} onClose={props.onCancel}>
      <DialogTitle>Select Mods</DialogTitle>
      <DialogContent>
        <Grid container>
          <Grid container direction="column" item xs>
            <Grid item>
              <IconButton onClick={props.onModUp} size="small">
                <ArrowUpwardIcon />
              </IconButton>
            </Grid>
            <Grid item>
              <IconButton onClick={props.onModDown} size="small">
                <ArrowDownwardIcon />
              </IconButton>
            </Grid>
          </Grid>
          <Grid item>
            {props.items && (
              <>
                <ModsList
                  items={props.items[0]}
                  onSelectMod={props.onSelectMod}
                  onToggleMod={props.onToggleMod}
                />
                <Divider />
                <ModsList
                  items={props.items[1]}
                  onSelectMod={props.onSelectMod}
                  onToggleMod={props.onToggleMod}
                />
              </>
            )}
          </Grid>
        </Grid>
      </DialogContent>
      <DialogActions>
        <Button onClick={props.onOk} color="primary">
          OK
        </Button>
        <Button onClick={props.onCancel}>Cancel</Button>
      </DialogActions>
    </Dialog>
  );
}
