import * as fs from "fs";
import * as path from "path";
import { choose } from "./util";

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
