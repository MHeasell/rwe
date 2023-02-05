import Registry from "winreg";

export enum HKEY {
  HKEY_CLASSES_ROOT = "HKEY_CLASSES_ROOT",
  HKEY_CURRENT_CONFIG = "HKEY_CURRENT_CONFIG",
  HKEY_DYN_DATA = "HKEY_DYN_DATA",
  HKEY_CURRENT_USER_LOCAL_SETTINGS = "HKEY_CURRENT_USER_LOCAL_SETTINGS",
  HKEY_CURRENT_USER = "HKEY_CURRENT_USER",
  HKEY_LOCAL_MACHINE = "HKEY_LOCAL_MACHINE",
  HKEY_PERFORMANCE_DATA = "HKEY_PERFORMANCE_DATA",
  HKEY_PERFORMANCE_TEXT = "HKEY_PERFORMANCE_TEXT",
  HKEY_PERFORMANCE_NLSTEXT = "HKEY_PERFORMANCE_NLSTEXT",
  HKEY_USERS = "HKEY_USERS",
}

export enum RegistryValueType {
  REG_BINARY = "REG_BINARY",
  REG_DWORD = "REG_DWORD",
  REG_DWORD_LITTLE_ENDIAN = "REG_DWORD_LITTLE_ENDIAN",
  REG_DWORD_BIG_ENDIAN = "REG_DWORD_BIG_ENDIAN",
  REG_EXPAND_SZ = "REG_EXPAND_SZ",
  REG_LINK = "REG_LINK",
  REG_MULTI_SZ = "REG_MULTI_SZ",
  REG_NONE = "REG_NONE",
  REG_QWORD = "REG_QWORD",
  REG_QWORD_LITTLE_ENDIAN = "REG_QWORD_LITTLE_ENDIAN",
  REG_SZ = "REG_SZ",
}

export type RegistryStringEntry = {
  readonly name: string;
  readonly type: RegistryValueType.REG_SZ | RegistryValueType.REG_EXPAND_SZ;
  readonly data: string;
};
export type RegistryNumberEntry = {
  readonly name: string;
  readonly type: RegistryValueType.REG_DWORD;
  readonly data: number;
};
export type RegistryValue = RegistryStringEntry | RegistryNumberEntry;

const ItemPattern = /^(.*)\s(REG_SZ|REG_MULTI_SZ|REG_EXPAND_SZ|REG_DWORD|REG_QWORD|REG_BINARY|REG_NONE)\s+([^\s].*)?$/;

function mapRegType(input: string): RegistryValueType | undefined {
  switch (input) {
    case "REG_BINARY":
      return RegistryValueType.REG_BINARY;
    case "REG_DWORD":
      return RegistryValueType.REG_DWORD;
    case "REG_DWORD_LITTLE_ENDIAN":
      return RegistryValueType.REG_DWORD_LITTLE_ENDIAN;
    case "REG_DWORD_BIG_ENDIAN":
      return RegistryValueType.REG_DWORD_BIG_ENDIAN;
    case "REG_EXPAND_SZ":
      return RegistryValueType.REG_EXPAND_SZ;
    case "REG_LINK":
      return RegistryValueType.REG_LINK;
    case "REG_MULTI_SZ":
      return RegistryValueType.REG_MULTI_SZ;
    case "REG_NONE":
      return RegistryValueType.REG_NONE;
    case "REG_QWORD":
      return RegistryValueType.REG_QWORD;
    case "REG_QWORD_LITTLE_ENDIAN":
      return RegistryValueType.REG_QWORD_LITTLE_ENDIAN;
    case "REG_SZ":
      return RegistryValueType.REG_SZ;
  }

  return undefined;
}

function mapHive(input: HKEY) {
  switch (input) {
    case HKEY.HKEY_CURRENT_USER:
      return Registry.HKCU;
    case HKEY.HKEY_LOCAL_MACHINE:
      return Registry.HKLM;
  }

  throw new Error("unsupported HKEY input");
}

/**
 * Note: only supports reading from HKCU and HKLM currently.
 * Note: only supports emitting REG_SZ entries currently.
 */
export function enumerateValues(
  hkey: HKEY,
  key: string
): Promise<RegistryValue[]> {
  const reg = new Registry({
    hive: mapHive(hkey),
    key: `\\${key}`,
  });
  return new Promise((resolve, reject) => {
    reg.values(function(err, items) {
      if (err) {
        console.log(
          `hkey: ${hkey}, key: ${key}, error: ${err.name}, message: ${err.message}`
        );
        resolve([]);
      } else {
        resolve(
          items.flatMap(item => {
            const regType = mapRegType(item.type);
            if (regType !== RegistryValueType.REG_SZ) {
              return [];
            }
            return {
              name: item.name,
              type: regType,
              data: item.value,
            };
          })
        );
      }
    });
  });
}
