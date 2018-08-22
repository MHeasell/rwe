import * as http from "http";
import * as socketio from "socket.io";
import { keepAliveRoom, createRoom, KeepAliveRoomRequest, deleteRoom } from "../web";
import * as protocol from "./protocol";

interface ServerObjects {
  localRoomId: number;
  httpServer: http.Server;
  ioServer: socketio.Server;
  nextPlayerId: number;
  players: protocol.PlayerInfo[];
  adminPlayerId?: number;
  roomInfo?: RoomInfo;
}

interface RoomInfo {
  id: number;
  key: string;
  intervalId: NodeJS.Timer;
}

// This function is a dirty hack to extract an IPv4 address
// out of an IPv4-mapped IPv6 address.
function extractAddress(addr: string) {
  const match = addr.match(/^::ffff:(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})$/);
  if (match) {
    return match[1];
  }
  return addr;
}

export class GameHostService {
  private nextLocalRoomId = 0;

  private server : ServerObjects | undefined;

  log(message: string) {
    console.log(`SERVER: ${message}`);
  }

  createServer(description: string, players: number, port: number = 1337) {
    if (this.server) {
      this.log("Server already running, destroying first");
      this.destroyServer();
    }
    this.log(`Creating server on port ${port}`)
    this.server = this.createServerObjects(port);

    this.server.ioServer.on("connection", socket => {
      const address = extractAddress(socket.handshake.address);
      if (!this.server) {
        this.log("Received new connection, but server not running!");
        return;
      }
      const playerId = this.server.nextPlayerId++;
      this.log(`Received new connection, assigned ID ${playerId}`);
      let info: protocol.PlayerInfo = {
        id: playerId,
        name: `Player #${playerId}`,
        host: address,
        side: "ARM",
        color: 0,
        team: 0,
        ready: false,
      };
      this.server.players.push(info);
      if (this.server.adminPlayerId === undefined) {
        this.server.adminPlayerId = playerId;
      }
      socket.on(protocol.Handshake, (data: protocol.HandshakePayload) => {
        if (!this.server) {
          this.log(`Received handshake from ${playerId}, but server not running!`);
          return;
        }
        this.log(`Received handshake from ${playerId}`);
        info.name = data.name;

        const handshakeResponse: protocol.HandshakeResponsePayload = {
          playerId: playerId,
          adminPlayerId: this.server.adminPlayerId!,
          players: this.server.players,
        };
        socket.emit(protocol.HandshakeResponse, handshakeResponse);

        const playerJoined: protocol.PlayerJoinedPayload = {
          playerId: playerId,
          name: data.name,
          host: address,
        };
        socket.broadcast.emit(protocol.PlayerJoined, playerJoined);

        socket.on(protocol.ChatMessage, (data: protocol.ChatMessagePayload) => {
          if (!this.server) {
            this.log(`Received chat-message from ${playerId}, but server not running!`);
            return;
          }
          const payload: protocol.PlayerChatMessagePayload = { playerId, message: data };
          this.server.ioServer.emit(protocol.PlayerChatMessage, payload);
        });
        socket.on(protocol.Ready, (data: protocol.ReadyPayload) => {
          if (!this.server) {
            this.log(`Received ready from ${playerId}, but server not running!`);
            return;
          }
          const payload: protocol.PlayerReadyPayload = { playerId, value: data };
          this.server.ioServer.emit(protocol.PlayerReady, payload);
        });
        socket.on(protocol.RequestStartGame, () => {
          if (!this.server) {
            this.log(`Received start-game from ${playerId}, but server not running!`);
            return;
          }
          if (playerId !== this.server.adminPlayerId) {
            this.log(`Received start-game from ${playerId}, but that player is not admin!`);
            return;
          }
          this.server.ioServer.emit(protocol.StartGame);
        });
        socket.on("disconnect", () => {
          if (!this.server) {
            this.log(`Player ${playerId} disconnected, but server not running!`);
            return;
          }
          this.server.players = this.server.players.filter(x => x.id !== playerId);
          if (this.server.adminPlayerId === playerId) {
            this.server.adminPlayerId = undefined;
          }
          const playerLeft: protocol.PlayerLeftPayload = {
            playerId: playerId
          };
          socket.broadcast.emit(protocol.PlayerLeft, playerLeft);
        });
      });
    });

    const localRoomId = this.server.localRoomId;
    this.publishRoom(port, description, players)
    .then(info => {
      if (!this.server || this.server.localRoomId != localRoomId) {
        clearInterval(info.intervalId);
        deleteRoom(info.id, { key: info.key });
        return;
      }

      this.server.roomInfo = info;
    });
  }

  destroyServer() {
    if (!this.server) { return; }
    this.server.ioServer.close();
    this.server.httpServer.close();
    if (this.server.roomInfo) {
      clearInterval(this.server.roomInfo.intervalId);
      deleteRoom(this.server.roomInfo.id, { key: this.server.roomInfo.key });
    }
    this.server = undefined;
  }

  private createServerObjects(port: number = 1337): ServerObjects {
    const server = http.createServer().listen(port);
    const io = socketio(server, { serveClient: false });

    return {
      localRoomId: this.nextLocalRoomId++,
      httpServer: server,
      ioServer: io,
      nextPlayerId: 1,
      players: [],
    };
  }

  private publishRoom(port: number, description: string, maxPlayers: number) {
    return createRoom({ port, description, number_of_players: 1, max_players: maxPlayers })
    .then(r => {
      const id = r.id;
      const key = r.key;
      const intervalId = setInterval(() => {
        if (!this.server) {
          this.log("Attempted to send keep-alive but server is not running!");
          return;
        }
        const payload: KeepAliveRoomRequest = {
          key,
          number_of_players: this.server.players.length,
        };
        keepAliveRoom(id, payload);
      }, 5000);

      const info: RoomInfo = { id, key: key, intervalId };
      return info;
    });
  }
}
