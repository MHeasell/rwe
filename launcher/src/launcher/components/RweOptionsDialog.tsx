import React from "react";
import {
  Dialog,
  DialogTitle,
  DialogContent,
  FormGroup,
  FormControlLabel,
  Checkbox,
  Select,
  MenuItem,
  FormControl,
  InputLabel,
  DialogActions,
  Button,
} from "@material-ui/core";
import { RweConfig } from "../rweConfig";

interface VideoMode {
  width: number;
  height: number;
}

type InterfaceMode = "left-click" | "right-click";

interface Props {
  videoModes: VideoMode[];
  initialFullscreen?: boolean;
  initiallySelectedMode?: VideoMode;
  initiallySelectedInterfaceMode?: InterfaceMode;

  onSubmit(options: RweConfig): void;
  onCancel(): void;
}

function formatVideoMode(m?: VideoMode) {
  if (!m) {
    return "Default";
  }
  return `${m.width}x${m.height}`;
}

function parseVideoMode(m: string): VideoMode | undefined {
  if (m === "Default") {
    return undefined;
  }
  const [w, h] = m.split("x");
  return { width: parseInt(w, 10), height: parseInt(h, 10) };
}

export function RweOptionsDialog(props: Props) {
  const [fullscreen, setFullscreen] = React.useState(!!props.initialFullscreen);
  const [videoMode, setVideoMode] = React.useState(props.initiallySelectedMode);
  const [interfaceMode, setInterfaceMode] = React.useState<InterfaceMode>(
    props.initiallySelectedInterfaceMode || "left-click"
  );

  return (
    <Dialog open>
      <DialogTitle>RWE Settings</DialogTitle>
      <DialogContent>
        <FormGroup row>
          <FormControlLabel
            control={
              <Checkbox
                checked={fullscreen}
                onChange={() => setFullscreen(!fullscreen)}
              />
            }
            label="Fullscreen"
          />
        </FormGroup>
        <FormGroup row>
          <FormControl>
            <InputLabel>Resolution</InputLabel>
            <Select
              value={formatVideoMode(videoMode)}
              onChange={e =>
                setVideoMode(parseVideoMode(e.target.value as string))
              }
            >
              <MenuItem key={formatVideoMode()} value={formatVideoMode()}>
                {formatVideoMode()}
              </MenuItem>
              {props.videoModes.map(m => (
                <MenuItem key={formatVideoMode(m)} value={formatVideoMode(m)}>
                  {formatVideoMode(m)}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </FormGroup>
        <FormGroup>
          <FormControl>
            <InputLabel>Interface Mode</InputLabel>
            <Select
              value={interfaceMode}
              onChange={e => setInterfaceMode(e.target.value as InterfaceMode)}
            >
              <MenuItem value="left-click">Left Click</MenuItem>
              <MenuItem value="right-click">Right Click</MenuItem>
            </Select>
          </FormControl>
        </FormGroup>
      </DialogContent>
      <DialogActions>
        <Button
          onClick={() =>
            props.onSubmit({
              fullscreen,
              width: videoMode && videoMode.width,
              height: videoMode && videoMode.height,
              interfaceMode: interfaceMode,
            })
          }
          color="primary"
        >
          OK
        </Button>
        <Button onClick={props.onCancel}>Cancel</Button>
      </DialogActions>
    </Dialog>
  );
}
