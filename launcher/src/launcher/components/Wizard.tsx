import React, { useState } from "react";
import {
  Dialog,
  Typography,
  CircularProgress,
  DialogActions,
  Button,
  DialogTitle,
  DialogContent,
  Grid,
} from "@material-ui/core";
import { assertNever } from "../../common/util";

export type State = "welcome" | "working" | "success" | "fail";
export interface WizardProps {
  state: State;
  onNext: (path?: string) => void;
  onClose: () => void;
}

export function Wizard(props: WizardProps) {
  const [filePath, setFilePath] = useState<string>();
  return (
    <Dialog open onClose={props.onClose}>
      <DialogTitle>Total Annihilation Mod Setup</DialogTitle>
      <DialogContent>
        <WizardContents
          state={props.state}
          path={filePath}
          onChange={p => {
            console.log(p);
            setFilePath(p);
          }}
        />
      </DialogContent>
      <DialogActions>
        {(props.state === "welcome" || props.state === "fail") && (
          <Button onClick={() => props.onNext(filePath)}>Next</Button>
        )}
        {props.state === "success" && (
          <Button onClick={() => props.onClose()}>Finish</Button>
        )}
      </DialogActions>
    </Dialog>
  );
}

function WizardContents(props: {
  state: State;
  path?: string;
  onChange: (path: string) => void;
}) {
  switch (props.state) {
    case "welcome":
      return <WizardWelcome />;
    case "working":
      return <WizardWorking />;
    case "success":
      return <WizardSuccess />;
    case "fail":
      return <WizardManual path={props.path} onChange={props.onChange} />;
  }
  assertNever(props.state);
}

function WizardWelcome() {
  return (
    <Typography>
      This wizard will automatically detect your existing installation of Total
      Annihilation and create a Total Annihilation mod for Robot War Engine.
    </Typography>
  );
}

function WizardWorking() {
  return (
    <Grid container direction="row" justify="center">
      <Grid item>
        <CircularProgress />
      </Grid>
    </Grid>
  );
}

function WizardSuccess() {
  return (
    <>
      <Typography>
        The wizard has successfully created the Total Annihilation mod.
      </Typography>
      <Typography>
        Please close and re-open the launcher in order to detect the new mod.
      </Typography>
    </>
  );
}

function WizardManual(props: {
  path?: string;
  onChange: (path: string) => void;
}) {
  return (
    <>
      <Typography>
        The wizard was unable to locate Total Annihilation game data.
      </Typography>
      <Typography>Manually locate your Total Annihilation folder:</Typography>
      <input
        type="file"
        // eslint-disable-next-line @typescript-eslint/ban-ts-ignore
        // @ts-ignore
        webkitdirectory="true"
        onChange={e => {
          props.onChange(e.target.files![0].path);
        }}
      />
    </>
  );
}
