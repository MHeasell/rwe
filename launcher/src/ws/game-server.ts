import * as http from "http";
import * as socketio from "socket.io";
import { keepAliveRoom, createRoom, KeepAliveRoomRequest, deleteRoom } from "../web";
import * as protocol from "./protocol";

interface RoomInfo {
  id: number;
  key: string;
  intervalId: NodeJS.Timer;
}

interface Room {
  localRoomId: number;
  nextPlayerId: number;
  players: protocol.PlayerInfo[];
  adminPlayerId?: number;
  roomInfo?: RoomInfo;
}

interface ServerObjects {
  port: number;
  httpServer: http.Server;
  ioServer: socketio.Server;
  rooms: Room[];
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
  private nextLocalRoomId = 1;

  private server: ServerObjects | undefined;

  log(message: string) {
    console.log(`SERVER: ${message}`);
  }

  createServer(port: number = 1337) {
    if (this.server) {
      this.log("Server already running, destroying first");
      this.destroyServer();
    }
    this.log(`Creating server on port ${port}`);
    this.server = this.createServerObjects(port);

    this.server.ioServer.on("connection", socket => {
      const address = extractAddress(socket.handshake.address);
      if (!this.server) {
        this.log("Received new connection, but server not running!");
        return;
      }
      socket.on(protocol.Handshake, (data: protocol.HandshakePayload) => {
        if (!this.server) {
          this.log(`Received handshake from ${address}, but server not running!`);
          return;
        }
        this.log(`Received handshake from ${address} with name "${data.name}" to join room ${data.roomId}`);

        const room = this.server.rooms.find(x => x.localRoomId === data.roomId);
        if (!room) {
          this.log(`Client from ${address} tried to join room ${data.roomId}, but it doesn't exist.`);
          return;
        }
        const roomLog = (msg: string) => this.log(`room ${room.localRoomId}: ${msg}`);
        const roomStr = `room/${room.localRoomId}`;

        const playerId = room.nextPlayerId++;
        roomLog(`Received new connection, assigned ID ${playerId}`);
        const info: protocol.PlayerInfo = {
          id: playerId,
          name: data.name,
          host: address,
          side: "ARM",
          color: 0,
          team: 0,
          ready: false,
        };

        room.players.push(info);
        if (room.adminPlayerId === undefined) {
          room.adminPlayerId = playerId;
        }

        socket.join(roomStr);

        const handshakeResponse: protocol.HandshakeResponsePayload = {
          playerId: playerId,
          adminPlayerId: room.adminPlayerId!,
          players: room.players,
        };
        socket.emit(protocol.HandshakeResponse, handshakeResponse);

        const playerJoined: protocol.PlayerJoinedPayload = {
          playerId: playerId,
          name: data.name,
          host: address,
        };
        socket.broadcast.to(roomStr).emit(protocol.PlayerJoined, playerJoined);

        socket.on(protocol.ChatMessage, (data: protocol.ChatMessagePayload) => {
          if (!this.server) {
            roomLog(`Received chat-message from ${playerId}, but server not running!`);
            return;
          }
          const payload: protocol.PlayerChatMessagePayload = { playerId, message: data };
          this.server.ioServer.to(roomStr).emit(protocol.PlayerChatMessage, payload);
        });
        socket.on(protocol.Ready, (data: protocol.ReadyPayload) => {
          if (!this.server) {
            roomLog(`Received ready from ${playerId}, but server not running!`);
            return;
          }
          const payload: protocol.PlayerReadyPayload = { playerId, value: data };
          this.server.ioServer.to(roomStr).emit(protocol.PlayerReady, payload);
        });
        socket.on(protocol.RequestStartGame, () => {
          if (!this.server) {
            roomLog(`Received start-game from ${playerId}, but server not running!`);
            return;
          }
          if (playerId !== room.adminPlayerId) {
            roomLog(`Received start-game from ${playerId}, but that player is not admin!`);
            return;
          }
          this.server.ioServer.to(roomStr).emit(protocol.StartGame);
        });
        socket.on("disconnect", () => {
          if (!this.server) {
            roomLog(`Player ${playerId} disconnected, but server not running!`);
            return;
          }
          room.players = room.players.filter(x => x.id !== playerId);
          if (room.adminPlayerId === playerId) {
            room.adminPlayerId = undefined;
          }
          const playerLeft: protocol.PlayerLeftPayload = {
            playerId,
          };
          socket.broadcast.to(roomStr).emit(protocol.PlayerLeft, playerLeft);
        });
      });
    });
  }

  createRoom(description: string, players: number): number | undefined {
    if (!this.server) {
      return undefined;
    }

    const room: Room = {
      localRoomId: this.nextLocalRoomId++,
      nextPlayerId: 1,
      players: [],
    };

    this.server.rooms.push(room);

    const localRoomId = room.localRoomId;
    this.publishRoom(localRoomId, this.server.port, description, players)
    .then(info => {
      if (!this.server || !this.server.rooms.find(x => x.localRoomId === localRoomId)) {
        clearInterval(info.intervalId);
        deleteRoom(info.id, { key: info.key });
        return;
      }

      room.roomInfo = info;
    });

    return localRoomId;
  }

  destroyRoom(roomId: number) {
    if (!this.server) { return; }
    const room = this.server.rooms.find(x => x.localRoomId === roomId);
    if (!room) { return; }
    if (room.roomInfo) {
      clearInterval(room.roomInfo.intervalId);
      deleteRoom(room.roomInfo.id, { key: room.roomInfo.key });
    }
    this.server.rooms = this.server.rooms.filter(x => x.localRoomId !== roomId);
  }

  destroyServer() {
    if (!this.server) { return; }
    this.server.ioServer.close();
    this.server.httpServer.close();
    for (const id of this.server.rooms.map(x => x.localRoomId)) {
      this.destroyRoom(id);
    }
    this.server = undefined;
  }

  private createServerObjects(port: number = 1337): ServerObjects {
    const server = http.createServer().listen(port);
    const io = socketio(server, { serveClient: false });

    return {
      port: port,
      httpServer: server,
      ioServer: io,
      rooms: [],
    };
  }

  private publishRoom(localRoomId: number, port: number, description: string, maxPlayers: number) {
    return createRoom({ port, local_room_id: localRoomId, description, number_of_players: 1, max_players: maxPlayers })
    .then(r => {
      const id = r.id;
      const key = r.key;
      const intervalId = setInterval(() => {
        if (!this.server) {
          this.log(`Attempted to send keep-alive for room ${localRoomId} but server is not running!`);
          return;
        }
        const room = this.server.rooms.find(x => x.localRoomId === localRoomId);
        if (!room) {
          this.log(`Attempted to send keep-alive for room ${localRoomId} but room does not exist!`);
          return;
        }
        const payload: KeepAliveRoomRequest = {
          key,
          number_of_players: room.players.length,
        };
        keepAliveRoom(id, payload);
      }, 5000);

      const info: RoomInfo = { id, key: key, intervalId };
      return info;
    });
  }
}
