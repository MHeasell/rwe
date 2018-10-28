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

interface GetMapListCommand {
  command: "map-list";
}

interface GetMapListResponse {
  result: "ok";
  maps: string[];
}

interface GetMinimapCommand {
  command: "get-minimap";
  map: string;
}

interface GetMinimapResponse {
  result: "ok";
  path: string;
}

interface CommandQueueItem {
  command: BridgeCommand;
  callback: (value: string) => void;
}

type BridgeCommand =
    AddDataPathCommand
  | ClearDataPathsCommand
  | GetMapInfoCommand
  | GetMapListCommand
  | GetMinimapCommand;

export class RweBridge {
  private readonly proc: ChildProcess;
  private readonly rl: readline.ReadLine;

  private readonly commandQueue: CommandQueueItem[] = [];

  private inProgressCommand?: CommandQueueItem;

  constructor() {
    const rweHome = process.env["RWE_HOME"];
    if (!rweHome) {
      throw new Error("Cannot launch rwe_bridge, RWE_HOME is not defined");
    }
    const bridgeExe = path.join(rweHome, "rwe_bridge.exe");

    this.proc = spawn(bridgeExe, undefined, { cwd: rweHome });

    this.rl = readline.createInterface({
      input: this.proc.stdout,
    });

    this.rl.on("line", line => this.onReceiveLine(line));
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

  getMapList(): Promise<GetMapListResponse> {
    const cmd: GetMapListCommand = { command: "map-list" };
    return this.submitCommand(cmd).then(answer => JSON.parse(answer) as GetMapListResponse);
  }

  getMinimap(mapName: string): Promise<GetMinimapResponse> {
    const cmd: GetMinimapCommand = { command: "get-minimap", map: mapName };
    return this.submitCommand(cmd).then(answer => JSON.parse(answer) as GetMinimapResponse);
  }

  private submitCommand(cmd: BridgeCommand): Promise<string> {
    return new Promise<string>((resolve, reject) => {
      this.commandQueue.push({command: cmd, callback: resolve});
      this.pumpCommands();
    });
  }

  private onReceiveLine(line: string) {
    console.log(`BRIDGE: received: ${line}`);

    if (!this.inProgressCommand) {
      console.warn("Received line when no command in progress!");
      return;
    }

    this.inProgressCommand.callback(line);
    this.inProgressCommand = undefined;
    this.pumpCommands();
  }

  private pumpCommands() {
    if (this.inProgressCommand) { return; }
    this.inProgressCommand = this.commandQueue.shift();
    if (!this.inProgressCommand) { return; }
    this.writeCommand(this.inProgressCommand.command);
  }

  private writeCommand(cmd: BridgeCommand) {
    const str = JSON.stringify(cmd);
    console.log(`BRIDGE: sending: ${str}`);
    this.proc.stdin.write(`${str}\n`);
  }
}
