import { execFile } from "child_process";
import * as path from "path";

export interface RweArgsPlayerHuman {
  type: "human";
}

export interface RweArgsPlayerComputer {
  type: "computer";
}

export interface RweArgsPlayerRemote {
  type: "remote";
  host: string;
  port: number;
}

export type RweArgsPlayerController = RweArgsPlayerHuman | RweArgsPlayerComputer | RweArgsPlayerRemote;

export interface RweArgsPlayerInfo {
  side: "ARM" | "CORE";
  color: number;
  controller: RweArgsPlayerController;
}

export interface RweArgs {
  dataPath?: string;
  map?: string;
  interface?: string;
  port?: number;
  players: RweArgsPlayerInfo[];
}

function serializeRweController(controller: RweArgsPlayerController): string {
  switch (controller.type) {
    case "human":
      return "Human";
    case "computer":
      return "Computer";
    case "remote":
      return `Network,${controller.host}:${controller.port.toString()}`;
    default:
      throw new Error("unknown controller type");
  }
}

function serializeRweArgs(args: RweArgs): string[] {
  const out = [];
  if (args.dataPath !== undefined) {
    out.push("--data-path", args.dataPath);
  }
  if (args.map !== undefined) {
    out.push("--map", args.map);
  }
  if (args.interface !== undefined) {
    out.push("--interface", args.interface);
  }
  if (args.port !== undefined) {
    out.push("--port", args.port.toString());
  }
  for (const p of args.players) {
    const controllerString = serializeRweController(p.controller);
    out.push("--player", `${controllerString};${p.side};${p.color}`);
  }
  return out;
}

// Naively quotes args, as if you were going to pass them through a shell,
// for display purposes.
// Don't use this for actual shell escaping, it's probably really insecure.
function quoteArg(arg: string) {
  if (arg.match(/[ "'\\]/)) {
    const escapedArg = arg.replace(/["\\]/, "\\$1");
    return `"${escapedArg}"`;
  }
  return arg;
}

export function execRwe(args?: RweArgs): Promise<any> {
  const rweHome = process.env["RWE_HOME"];
  if (!rweHome) {
    return Promise.reject("Cannot launch RWE, RWE_HOME is not defined");
  }

  const serializedArgs = args ? serializeRweArgs(args) : undefined;

  return new Promise((resolve, reject) => {
    if (serializedArgs) {
      console.log("Launching RWE with args: " + serializedArgs.map(quoteArg).join(" "));
    }
    else {
      console.log("Launching RWE");
    }
    // FIXME: assumes windows
    execFile(path.join(rweHome, "rwe.exe"), serializedArgs, { cwd: rweHome }, (error: (null | Error), stdout: any, stderr: any) => {
      if (error) {
        const exitCode = (error as any).code;
        reject(`RWE exited with exit code ${exitCode}: ${error.message}`);
        return;
      }

      resolve();
    });
  });
}
