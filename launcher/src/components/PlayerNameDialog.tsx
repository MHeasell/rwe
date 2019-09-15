import {
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogContentText,
  TextField,
} from "@material-ui/core";
import * as React from "react";

interface PlayerNameDialogProps {
  open: boolean;
  onClose: () => void;
  onConfirm: (name: string) => void;
}
interface PlayerNameDialogState {
  name: string;
}

export class PlayerNameDialog extends React.Component<
  PlayerNameDialogProps,
  PlayerNameDialogState
> {
  constructor(props: PlayerNameDialogProps) {
    super(props);
    this.state = { name: "" };

    this.handleConfirm = this.handleConfirm.bind(this);
    this.handleChange = this.handleChange.bind(this);
    this.handleClose = this.handleClose.bind(this);
  }

  handleChange(event: React.SyntheticEvent<EventTarget>) {
    const value = (event.target as HTMLInputElement).value;
    this.setState({ name: value });
  }

  handleConfirm(event: React.SyntheticEvent<EventTarget>) {
    event.preventDefault();
    this.props.onConfirm(this.state.name);
  }

  handleClose() {
    this.setState({ name: "" });
    this.props.onClose();
  }

  render() {
    return (
      <Dialog open={this.props.open} onClose={this.handleClose}>
        <form onSubmit={this.handleConfirm}>
          <DialogContent>
            <DialogContentText>Enter your player name</DialogContentText>
            <TextField autoFocus label="name" onChange={this.handleChange} />
          </DialogContent>
          <DialogActions>
            <Button color="primary" type="submit">
              Join
            </Button>
            <Button onClick={this.handleClose}>Cancel</Button>
          </DialogActions>
        </form>
      </Dialog>
    );
  }
}
