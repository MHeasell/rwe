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
