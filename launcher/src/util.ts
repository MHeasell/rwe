import * as fs from "fs";
import * as path from "path";

export function masterServer() {
  if (process.env["RWE_MASTER_SERVER"]) {
    return process.env["RWE_MASTER_SERVER"];
  }
  return "https://master.rwe.michaelheasell.com";
}

export function getAddr(socket: SocketIO.Socket, reverseProxy: boolean) {
  // If we are running behind a reverse proxy,
  // we need to look at x-forwarded-for
  // to get the correct remote address.
  if (reverseProxy) {
    const addrs: string = socket.handshake.headers["x-forwarded-for"];
    const addrsList = addrs.split(", ");
    return addrsList[addrsList.length - 1];
  }
  return socket.handshake.address;
}

export function findAndMap<T, R>(
  arr: T[],
  f: (x: T) => R | undefined
): R | undefined {
  for (const e of arr) {
    const v = f(e);
    if (v !== undefined) {
      return v;
    }
  }
  return undefined;
}

export function choose<T, R>(arr: T[], f: (x: T) => R | undefined): R[] {
  const out: R[] = [];
  for (const e of arr) {
    const v = f(e);
    if (v !== undefined) {
      out.push(v);
    }
  }
  return out;
}

export function assertNever(x: never): never {
  throw new Error(`Unexpected object: ${x}`);
}

export interface InstalledModResult {
  name: string;
  path: string;
}
export function getInstalledMods(baseDir: string): InstalledModResult[] {
  const entries = fs
    .readdirSync(baseDir, { withFileTypes: true })
    .filter(e => e.isDirectory())
    .map<InstalledModResult | undefined>(e => {
      const modPath = path.join(baseDir, e.name);
      const json = readRweModJson(path.join(modPath, "rwe_mod.json"));
      if (!json) {
        return undefined;
      }
      return {
        name: json.shortname,
        path: modPath,
      };
    });

  return choose(entries, x => x);
}

export interface RweModJson {
  shortname: string;
  name: string;
  author: string;
}

function readRweModJson(path: string): RweModJson | undefined {
  let buffer: Buffer;
  try {
    buffer = fs.readFileSync(path);
  } catch (err) {
    return undefined;
  }
  return JSON.parse(buffer.toString());
}

export function toggleItem<T>(arr: T[], item: T): T[] {
  return arr.includes(item) ? arr.filter(x => x !== item) : [...arr, item];
}

export function moveUp<T>(arr: T[], item: T): T[] {
  const index = arr.indexOf(item);
  if (index === -1) {
    return arr;
  }
  if (index === 0) {
    return arr;
  }
  return [
    ...arr.slice(0, index - 1),
    arr[index],
    arr[index - 1],
    ...arr.slice(index + 1),
  ];
}

export function moveDown<T>(arr: T[], item: T): T[] {
  const index = arr.indexOf(item);
  if (index === -1) {
    return arr;
  }
  if (index === arr.length - 1) {
    return arr;
  }
  return [
    ...arr.slice(0, index),
    arr[index + 1],
    arr[index],
    ...arr.slice(index + 2),
  ];
}
