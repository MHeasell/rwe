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
import { toggleItem, moveUp, moveDown } from "../../common/util";

export interface ItemProps {
  name: string;
  checked: boolean;
  selected: boolean;
  onSelect(): void;
  onToggle(): void;
}

function ModListItem(props: ItemProps) {
  return (
    <ListItem dense button selected={props.selected} onClick={props.onSelect}>
      <ListItemText primary={props.name} />
      <ListItemSecondaryAction>
        <Checkbox checked={props.checked} onChange={props.onToggle} />
      </ListItemSecondaryAction>
    </ListItem>
  );
}

interface Item {
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
          <ModListItem
            key={item.name}
            name={item.name}
            checked={item.checked}
            selected={item.selected}
            onSelect={() => props.onSelectMod(item.name)}
            onToggle={() => props.onToggleMod(item.name)}
          />
        );
      })}
    </List>
  );
}

export interface Props {
  title: string;
  items: string[];
  initiallyActiveItems: string[];
  onSubmit(selectedItems: string[]): void;
  onCancel(): void;
}

export function SelectModsDialog(props: Props) {
  const [state, dispatch] = React.useReducer(modDialogReducer, {
    activeItems: props.initiallyActiveItems,
  });
  const active = state.activeItems.map<Item>(x => ({
    name: x,
    checked: true,
    selected: x === state.selectedItem,
  }));
  const inactive = props.items
    .filter(x => !state.activeItems.includes(x))
    .map<Item>(x => ({
      name: x,
      checked: false,
      selected: x === state.selectedItem,
    }));
  return (
    <Dialog open onClose={props.onCancel}>
      <DialogTitle>{props.title}</DialogTitle>
      <DialogContent>
        <Grid container>
          <Grid container direction="column" item xs>
            <Grid item>
              <IconButton
                onClick={() => dispatch({ type: "MOD_UP" })}
                size="small"
              >
                <ArrowUpwardIcon />
              </IconButton>
            </Grid>
            <Grid item>
              <IconButton
                onClick={() => dispatch({ type: "MOD_DOWN" })}
                size="small"
              >
                <ArrowDownwardIcon />
              </IconButton>
            </Grid>
          </Grid>
          <Grid item>
            {props.items && (
              <>
                <ModsList
                  items={active}
                  onSelectMod={name => dispatch({ type: "SELECT_MOD", name })}
                  onToggleMod={name => dispatch({ type: "TOGGLE_MOD", name })}
                />
                <Divider />
                <ModsList
                  items={inactive}
                  onSelectMod={name => dispatch({ type: "SELECT_MOD", name })}
                  onToggleMod={name => dispatch({ type: "TOGGLE_MOD", name })}
                />
              </>
            )}
          </Grid>
        </Grid>
      </DialogContent>
      <DialogActions>
        <Button
          onClick={() => props.onSubmit(state.activeItems)}
          color="primary"
        >
          OK
        </Button>
        <Button onClick={props.onCancel}>Cancel</Button>
      </DialogActions>
    </Dialog>
  );
}

interface ModsDialogState {
  activeItems: string[];
  selectedItem?: string;
}

export interface SelectModAction {
  type: "SELECT_MOD";
  name: string;
}
export interface ToggleModAction {
  type: "TOGGLE_MOD";
  name: string;
}
export interface ModUpAction {
  type: "MOD_UP";
}
export interface ModDownAction {
  type: "MOD_DOWN";
}
export type ModsDialogAction =
  | SelectModAction
  | ToggleModAction
  | ModUpAction
  | ModDownAction;

function modDialogReducer(
  modsDialog: ModsDialogState,
  action: ModsDialogAction
): ModsDialogState {
  switch (action.type) {
    case "SELECT_MOD": {
      return { ...modsDialog, selectedItem: action.name };
    }
    case "TOGGLE_MOD": {
      const newList = toggleItem(modsDialog.activeItems, action.name);
      return { ...modsDialog, activeItems: newList };
    }
    case "MOD_UP": {
      if (!modsDialog.selectedItem) {
        return modsDialog;
      }
      const newList = moveUp(modsDialog.activeItems, modsDialog.selectedItem);
      return { ...modsDialog, activeItems: newList };
    }
    case "MOD_DOWN": {
      if (!modsDialog.selectedItem) {
        return modsDialog;
      }
      const newList = moveDown(modsDialog.activeItems, modsDialog.selectedItem);
      return { ...modsDialog, activeItems: newList };
    }

    default:
      return modsDialog;
  }
}
