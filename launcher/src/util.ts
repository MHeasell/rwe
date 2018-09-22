export function masterServer() {
  if (process.env["RWE_MASTER_SERVER"]) {
    return process.env["RWE_MASTER_SERVER"];
  }
  return "https://master.rwe.michaelheasell.com";
}
