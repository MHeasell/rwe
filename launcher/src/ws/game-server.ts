import * as protocol from "./protocol";
import * as rx from "rxjs";
import * as crypto from "crypto";

export interface AdminUnclaimed {
  state: "unclaimed";
  adminKey: string;
}

export interface AdminClaimed {
  state: "claimed";
  adminPlayerId?: number;
}

export type AdminState = AdminUnclaimed | AdminClaimed;

export interface Room {
  description: string;
  nextPlayerId: number;
  players: protocol.PlayerInfo[];
  maxPlayers: number;
  adminState: AdminState;
}

function generateAdminKey() {
  return crypto.randomBytes(16).toString("hex");
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

export interface GameCreatedInfo {
  gameId: number;
  adminKey: string;
}

export class GameServer {
  private readonly ns: SocketIO.Namespace;

  private nextRoomId = 1;
  private readonly rooms = new Map<number, Room>();

  private _gameUpdated = new rx.Subject<[number, Room]>();
  private _gameDeleted = new rx.Subject<number>();

  constructor (ns: SocketIO.Namespace) {
    this.ns = ns;
    this.connect();
  }

  get gameUpdated(): rx.Observable<[number, Room]> { return this._gameUpdated; }
  get gameDeleted(): rx.Observable<number> { return this._gameDeleted; }

  getAllRooms() {
    return this.rooms.entries();
  }

  createRoom(description: string, maxPlayers: number): GameCreatedInfo {
    const id = this.nextRoomId++;
    const adminKey = generateAdminKey();
    this.rooms.set(id, {
      description,
      nextPlayerId: 1,
      players: [],
      maxPlayers,
      adminState: { state: "unclaimed", adminKey },
    });

    return { gameId: id, adminKey };
  }

  deleteRoom(id: number) {
    if (this.rooms.delete(id)) {
      this._gameDeleted.next(id);
      return true;
    }
    return false;
  }

  getAdminKey(id: number) {
    const room = this.rooms.get(id);
    if (!room) { return undefined; }
    return room.adminState.state === "unclaimed" ? room.adminState.adminKey : undefined;
  }

  getRoomInfo(id: number) {
    return this.rooms.get(id);
  }

  getNumberOfPlayers(id: number) {
    const room = this.rooms.get(id);
    if (!room) { return undefined; }
    return room.players.length;
  }

  connect() {
    this.ns.on("connection", socket => {
      const address = extractAddress(socket.handshake.address);
      this.log(`Received connection from ${address}`);
      socket.on(protocol.Handshake, (data: protocol.HandshakePayload) => {
        this.log(`Received handshake from ${address} with name "${data.name}"`);

        const roomId = data.gameId;
        const room = this.rooms.get(roomId);
        if (!room) {
          this.log(`Received handshake to connect to room ${data.gameId}, but room does not exist`);
          socket.disconnect();
          return;
        }

        const playerId = room.nextPlayerId++;
        this.log(`Received new connection, assigned ID ${playerId}`);

        room.players.push({
          id: playerId,
          name: data.name,
          host: address,
          side: "ARM",
          color: 0,
          team: 0,
          ready: false,
        });

        if (room.adminState.state === "unclaimed") {
          if (data.adminKey === room.adminState.adminKey) {
            room.adminState = { state: "claimed", adminPlayerId: playerId };
          }
        }

        const roomString = `room/${roomId}`;
        socket.join(roomString);

        this._gameUpdated.next([roomId, room]);

        const handshakeResponse: protocol.HandshakeResponsePayload = {
          playerId: playerId,
          adminPlayerId: room.adminState.state === "claimed" ? room.adminState.adminPlayerId : undefined,
          players: room.players,
        };
        socket.emit(protocol.HandshakeResponse, handshakeResponse);

        const playerJoined: protocol.PlayerJoinedPayload = {
          playerId: playerId,
          name: data.name,
          host: address,
        };
        socket.broadcast.emit(protocol.PlayerJoined, playerJoined);

        socket.on(protocol.ChatMessage, (data: protocol.ChatMessagePayload) => {
          this.onChatMessage(roomId, playerId, data);
        });
        socket.on(protocol.Ready, (data: protocol.ReadyPayload) => {
          this.onPlayerReady(roomId, playerId, data);
        });
        socket.on(protocol.RequestStartGame, () => {
          this.onPlayerRequestStartGame(roomId, playerId);
        });
        socket.on("disconnect", () => {
          this.onDisconnected(roomId, playerId);
        });
      });
    });
  }

  private log(message: string) {
    console.log(`game server: ${message}`);
  }

  private sendToRoom(roomId: number, event: string, ...args: any[]) {
    const roomString = `room/${roomId}`;
    this.ns.to(roomString).emit(event, ...args);
  }

  private onChatMessage(roomId: number, playerId: number, message: string) {
    const payload: protocol.PlayerChatMessagePayload = { playerId, message };
    this.sendToRoom(roomId, protocol.PlayerChatMessage, payload);
  }

  private onPlayerReady(roomId: number, playerId: number, value: boolean) {
    const payload: protocol.PlayerReadyPayload = { playerId, value };
    this.sendToRoom(roomId, protocol.PlayerReady, payload);
  }

  private onPlayerRequestStartGame(roomId: number, playerId: number) {
    const room = this.rooms.get(roomId);
    if (!room) { throw new Error("onPlayerRequestStartGame triggered for non-existent room"); }
    if (room.adminState.state !== "claimed" || room.adminState.adminPlayerId !== playerId) {
      this.log(`Received start-game from ${playerId}, but that player is not admin!`);
      return;
    }
    this.sendToRoom(roomId, protocol.StartGame);
  }

  private onDisconnected(roomId: number, playerId: number) {
    const room = this.rooms.get(roomId);
    if (!room) { throw new Error("onDisconnected triggered for non-existent room"); }
    room.players = room.players.filter(x => x.id !== playerId);
    const playerLeft: protocol.PlayerLeftPayload = {
      playerId,
    };
    if (room.adminState.state === "claimed" && room.adminState.adminPlayerId === playerId) {
      if (room.players.length < 1) {
        room.adminState.adminPlayerId = undefined;
      }
      else {
        room.adminState.adminPlayerId = room.players[0].id;
        playerLeft.newAdminPlayerId = room.adminState.adminPlayerId;
      }
    }
    this.sendToRoom(roomId, protocol.PlayerLeft, playerLeft);
    this._gameUpdated.next([roomId, room]);

    if (room.players.length === 0) {
      this.deleteRoom(roomId);
    }
  }
}
