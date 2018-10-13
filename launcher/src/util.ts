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

export function findAndMap<T, R>(arr: T[], f: (x: T) => (R | undefined)): (R | undefined) {
  for (const e of arr) {
    const v = f(e);
    if (v !== undefined) { return v; }
  }
  return undefined;
}

export function assertNever(x: never): never {
  throw new Error(`Unexpected object: ${x}`);
}
