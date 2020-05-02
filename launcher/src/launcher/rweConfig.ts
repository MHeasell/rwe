import os from "os";
import fs from "fs";
import path from "path";

export interface RweConfig {
  width?: number;
  height?: number;
  fullscreen?: boolean;
  interfaceMode?: "left-click" | "right-click";
}

function parseConfigBoolean(b?: string): boolean | undefined {
  if (b === "true") {
    return true;
  }
  if (b === "false") {
    return false;
  }
  return undefined;
}

function parseConfigInt(n?: string): number | undefined {
  if (!n) {
    return undefined;
  }
  return parseInt(n, 10);
}

function parseConfigInterfaceMode(
  m?: string
): "left-click" | "right-click" | undefined {
  if (m === "left-click") {
    return "left-click";
  }
  if (m === "right-click") {
    return "right-click";
  }

  return undefined;
}

export function readConfigFromFile(filename: string): Promise<RweConfig> {
  return new Promise((resolve, reject) => {
    fs.readFile(filename, "utf8", (err, data) => {
      if (err) {
        if (err.code === "ENOENT") {
          resolve({});
        } else {
          reject(err);
        }
        return;
      }
      resolve(readConfig(data));
    });
  });
}

export function readConfig(lines: string): RweConfig {
  const m = readConfigMap(lines);
  return {
    width: parseConfigInt(m.get("width")),
    height: parseConfigInt(m.get("height")),
    fullscreen: parseConfigBoolean(m.get("fullscreen")),
    interfaceMode: parseConfigInterfaceMode(m.get("interface-mode")),
  };
}

export function writeConfigToFile(
  filename: string,
  data: RweConfig
): Promise<void> {
  return new Promise((resolve, reject) => {
    fs.mkdirSync(path.dirname(filename), { recursive: true });
    fs.writeFile(filename, writeConfig(data), "utf8", err => {
      if (err) {
        reject(err);
        return;
      }
      resolve();
    });
  });
}

export function writeConfig(config: RweConfig): string {
  const arr: [string, string][] = [];

  if (config.width) {
    arr.push(["width", config.width.toString()]);
  }
  if (config.height) {
    arr.push(["height", config.height.toString()]);
  }
  if (config.fullscreen) {
    arr.push(["fullscreen", "true"]);
  }
  if (config.interfaceMode) {
    arr.push(["interface-mode", config.interfaceMode]);
  }

  return arr.map(x => `${x[0]} = ${x[1]}${os.EOL}`).join("");
}

function readConfigMap(lines: string): Map<string, string> {
  const m = new Map<string, string>();
  for (const line of lines.split(/\r?\n/)) {
    const match = line.match(/^\s*([\w-]+)\s*=\s*([\w-]+)\s*$/);
    if (!match) {
      continue;
    }
    m.set(match[1], match[2]);
  }
  return m;
}
