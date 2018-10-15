import { spawn, ChildProcess } from "child_process";
import * as path from "path";
import * as readline from "readline";

interface SuccessResponse {
  result: "ok";
}

interface AddDataPathCommand {
  command: "add-data-path";
  path: string;
}

type AddDataPathResponse = SuccessResponse;

interface ClearDataPathsCommand {
  command: "clear-data-paths";
}

type ClearDataPathsResponse = SuccessResponse;

interface GetMapInfoCommand {
  command: "map-info";
  map: string;
}

interface GetMapInfoResponse {
  result: "ok";
  data: string;
}

interface CommandQueueItem {
  command: BridgeCommand;
  callback: (value: string) => void;
}

type BridgeCommand =
    AddDataPathCommand
  | ClearDataPathsCommand
  | GetMapInfoCommand;

export class RweBridge {
  private readonly proc: ChildProcess;
  private readonly rl: readline.ReadLine;

  private readonly commandQueue: CommandQueueItem[] = [];

  private isCommandInProgress = false;

  constructor() {
    const rweHome = process.env["RWE_HOME"];
    if (!rweHome) {
      throw new Error("Cannot launch rwe_bridge, RWE_HOME is not defined");
    }
    const bridgeExe = path.join(rweHome, "rwe_bridge.exe");

    this.proc = spawn(bridgeExe, undefined, { cwd: rweHome });

    this.rl = readline.createInterface({
      input: this.proc.stdout,
      output: this.proc.stdin,
    });
  }

  addDataPath(path: string): Promise<AddDataPathResponse> {
    const cmd: AddDataPathCommand = { command: "add-data-path", path };
    return this.submitCommand(cmd).then(answer => JSON.parse(answer) as AddDataPathResponse);
  }

  clearDataPaths(): Promise<ClearDataPathsResponse> {
    const cmd: ClearDataPathsCommand = { command: "clear-data-paths" };
    return this.submitCommand(cmd).then(answer => JSON.parse(answer) as ClearDataPathsResponse);
  }

  getMapInfo(mapName: string): Promise<GetMapInfoResponse> {
    const cmd: GetMapInfoCommand = { command: "map-info", map: mapName };
    return this.submitCommand(cmd).then(answer => JSON.parse(answer) as GetMapInfoResponse);
  }

  private submitCommand(cmd: BridgeCommand): Promise<string> {
    return new Promise<string>((resolve, reject) => {
      this.commandQueue.push({command: cmd, callback: resolve});
      this.pumpCommands();
    });
  }

  private pumpCommands() {
    if (this.isCommandInProgress) { return; }

    const cmd = this.commandQueue.shift();
    if (!cmd) { return; }

    this.isCommandInProgress = true;
    this.writeCommand(cmd.command)
    .then(answer => {
      cmd.callback(answer);
      this.isCommandInProgress = false;
      this.pumpCommands();
    });
  }

  private writeCommand(cmd: BridgeCommand): Promise<string> {
    const str = JSON.stringify(cmd);
    return new Promise<string>((resolve, reject) => {
      this.rl.question(str, answer => { resolve(answer); });
    });
  }
}
