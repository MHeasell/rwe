import os from "os";

export interface RweConfig {
  width?: number;
  height?: number;
  fullscreen?: boolean;
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

export function readConfig(lines: string): RweConfig {
  const m = readConfigMap(lines);
  return {
    width: parseConfigInt(m.get("width")),
    height: parseConfigInt(m.get("height")),
    fullscreen: parseConfigBoolean(m.get("fullscreen")),
  };
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

  return arr.map(x => `${x[0]} = ${x[1]}${os.EOL}`).join("");
}

function readConfigMap(lines: string): Map<string, string> {
  const m = new Map<string, string>();
  for (const line of lines.split(/\r?\n/)) {
    const match = line.match(/^\s*(\w+)\s*=\s*(\w+)\s*$/);
    if (!match) {
      continue;
    }
    m.set(match[0], match[1]);
  }
  return m;
}
