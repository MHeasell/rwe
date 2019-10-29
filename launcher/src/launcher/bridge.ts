import { ChildProcess, spawn } from "child_process";
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

export interface GetMapInfoResponse {
  result: "ok";
  description: string;
  memory: string;
  numberOfPlayers: string;
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

interface GetVideoModesCommand {
  command: "video-modes";
}

interface GetVideoModesResponse {
  modes: { width: number; height: number }[];
}

interface CommandQueueItem {
  command: BridgeCommand;
  callback: (value: string) => void;
}

type BridgeCommand =
  | AddDataPathCommand
  | ClearDataPathsCommand
  | GetMapInfoCommand
  | GetMapListCommand
  | GetMinimapCommand
  | GetVideoModesCommand;

export class RweBridge {
  private readonly proc: ChildProcess;
  private readonly rl: readline.ReadLine;

  private readonly commandQueue: CommandQueueItem[] = [];

  constructor() {
    const rweHome = process.env["RWE_HOME"];
    if (!rweHome) {
      throw new Error("Cannot launch rwe_bridge, RWE_HOME is not defined");
    }
    const bridgeExe = path.join(
      rweHome,
      "rwe_bridge" + (process.platform === "win32" ? ".exe" : "")
    );

    this.proc = spawn(bridgeExe, undefined, { cwd: rweHome });

    this.rl = readline.createInterface({
      input: this.proc.stdout!,
    });

    this.rl.on("line", line => this.onReceiveLine(line));
  }

  addDataPath(path: string): Promise<AddDataPathResponse> {
    const cmd: AddDataPathCommand = { command: "add-data-path", path };
    return this.submitCommand(cmd).then(
      answer => JSON.parse(answer) as AddDataPathResponse
    );
  }

  clearDataPaths(): Promise<ClearDataPathsResponse> {
    const cmd: ClearDataPathsCommand = { command: "clear-data-paths" };
    return this.submitCommand(cmd).then(
      answer => JSON.parse(answer) as ClearDataPathsResponse
    );
  }

  getMapInfo(mapName: string): Promise<GetMapInfoResponse> {
    const cmd: GetMapInfoCommand = { command: "map-info", map: mapName };
    return this.submitCommand(cmd).then(
      answer => JSON.parse(answer) as GetMapInfoResponse
    );
  }

  getMapList(): Promise<GetMapListResponse> {
    const cmd: GetMapListCommand = { command: "map-list" };
    return this.submitCommand(cmd).then(
      answer => JSON.parse(answer) as GetMapListResponse
    );
  }

  getMinimap(mapName: string): Promise<GetMinimapResponse> {
    const cmd: GetMinimapCommand = { command: "get-minimap", map: mapName };
    return this.submitCommand(cmd).then(
      answer => JSON.parse(answer) as GetMinimapResponse
    );
  }

  getVideoModes(): Promise<GetVideoModesResponse> {
    const cmd: GetVideoModesCommand = { command: "video-modes" };
    return this.submitCommand(cmd).then(
      answer => JSON.parse(answer) as GetVideoModesResponse
    );
  }

  private submitCommand(cmd: BridgeCommand): Promise<string> {
    return new Promise<string>((resolve, reject) => {
      if (this.commandQueue.length === 0) {
        this.writeCommand(cmd);
      }
      this.commandQueue.push({ command: cmd, callback: resolve });
    });
  }

  private onReceiveLine(line: string) {
    console.log(`BRIDGE: received: ${line}`);

    const item = this.commandQueue.shift();
    if (!item) {
      console.warn("Received line when no command in progress!");
      return;
    }

    item.callback(line);

    if (this.commandQueue.length === 0) {
      return;
    }
    this.writeCommand(this.commandQueue[0].command);
  }

  private writeCommand(cmd: BridgeCommand) {
    const str = JSON.stringify(cmd);
    console.log(`BRIDGE: sending: ${str}`);
    this.proc.stdin!.write(`${str}\n`);
  }
}
