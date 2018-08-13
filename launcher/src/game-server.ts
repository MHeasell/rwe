import * as http from "http";
import * as socketio from "socket.io";
import * as socketioClient from "socket.io-client";
import { Store, MiddlewareAPI, Dispatch } from "redux";
import { State } from "./reducers";
import { receiveChatMessage, receiveHandshakeResponse, receivePlayerJoined, receivePlayerLeft, disconnectGame, receivePlayerReady } from "./actions";
import { keepAliveRoom, createRoom, KeepAliveRoomRequest, deleteRoom } from "./web";

type PlayerSide = "ARM" | "CORE";
type PlayerColor = number;

export interface PlayerInfo {
  id: number;
  name: string;
  side: PlayerSide;
  color: PlayerColor;
  team: number;
  ready: boolean;
}

export interface HandshakePayload {
  name: string;
}

export interface HandshakeResponsePayload {
  playerId: number;
  players: PlayerInfo[];
}

export type ChatMessagePayload = string;

export interface PlayerChatMessagePayload {
  playerId: number;
  message: string;
}

export interface PlayerJoinedPayload {
  playerId: number;
  name: string;
}

export interface PlayerLeftPayload {
  playerId: number;
}

export interface PlayerReadyPayload {
  playerId: number;
  value: boolean;
}

export class GameClientService {
  private readonly store : MiddlewareAPI<Dispatch, State>;
  private client : SocketIOClient.Socket | undefined;

  constructor(store: MiddlewareAPI<Dispatch, State>) {
    this.store = store;
  }

  connectToServer(connectionString: string, playerName: string) {
    this.client = socketioClient(connectionString);

    this.client.on("connect_error", (error: any) => {
      console.log(error);
    });

    this.client.on("disconnect", () => {
      this.disconnect();
      this.store.dispatch(disconnectGame());
    });

    const handshake: HandshakePayload = {
      name: playerName,
    };
    this.client.emit("handshake", handshake);

    this.client.on("handshake-response", (data: HandshakeResponsePayload) => {
      this.store.dispatch(receiveHandshakeResponse(data));
    });

    this.client.on("player-joined", (data: PlayerJoinedPayload) => {
      this.store.dispatch(receivePlayerJoined(data));
    });

    this.client.on("player-left", (data: PlayerLeftPayload) => {
      this.store.dispatch(receivePlayerLeft(data));
    });

    this.client.on("player-chat-message", (data: PlayerChatMessagePayload) => {
      this.store.dispatch(receiveChatMessage(data));
    });

    this.client.on("player-ready", (data: PlayerReadyPayload) => {
      this.store.dispatch(receivePlayerReady(data));
    });
  }

  disconnect() {
    if (!this.client) { return; }
    this.client.close();
    this.client = undefined;
  }

  sendChatMessage(message: string) {
    if (!this.client) { return; }
    this.client.emit("chat-message", message);
  }

  setReadyState(value: boolean) {
    if (!this.client) { return; }
    this.client.emit("ready", value);
  }
}

interface ServerObjects {
  localRoomId: number;
  httpServer: http.Server;
  ioServer: socketio.Server;
  nextPlayerId: number;
  players: PlayerInfo[];
  roomInfo?: RoomInfo;
}

interface RoomInfo {
  id: number;
  key: string;
  intervalId: NodeJS.Timer;
}

function publishRoom(port: number, description: string, maxPlayers: number) {
  return createRoom({ port, description, number_of_players: 1, max_players: maxPlayers })
  .then(r => {
    const id = r.id;
    const payload: KeepAliveRoomRequest = { key: r.key, number_of_players: 1 }; // FIXME: hardcoded number_of_players

    const intervalId = setInterval(() => {
      keepAliveRoom(id, payload);
    }, 5000);

    const info: RoomInfo = { id, key: r.key, intervalId };
    return info;
  });
}

export class GameHostService {
  private nextLocalRoomId = 0;

  private readonly store : MiddlewareAPI<Dispatch, State>;

  private server : ServerObjects | undefined;

  constructor(store: MiddlewareAPI<Dispatch, State>) {
    this.store = store;
  }

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
      if (!this.server) {
        this.log("Received new connection, but server not running!");
        return;
      }
      const playerId = this.server.nextPlayerId++;
      this.log(`Received new connection, assigned ID ${playerId}`);
      let info: PlayerInfo = {
        id: playerId,
        name: `Player #${playerId}`,
        side: "ARM",
        color: 0,
        team: 0,
        ready: false,
      };
      this.server.players.push(info);
      socket.on("handshake", (data: HandshakePayload) => {
        if (!this.server) {
          this.log(`Received handshake from ${playerId}, but server not running!`);
          return;
        }
        this.log(`Received handshake from ${playerId}`);
        info.name = data.name;

        const handshakeResponse: HandshakeResponsePayload = {
          playerId: playerId,
          players: this.server.players,
        };
        socket.emit("handshake-response", handshakeResponse);

        const playerJoined: PlayerJoinedPayload = {
          playerId: playerId,
          name: data.name,
        };
        socket.broadcast.emit("player-joined", playerJoined);

        socket.on("chat-message", (data: ChatMessagePayload) => {
          if (!this.server) {
            this.log(`Received chat-message from ${playerId}, but server not running!`);
            return;
          }
          const payload: PlayerChatMessagePayload = { playerId, message: data };
          this.server.ioServer.emit("player-chat-message", payload);
        });
        socket.on("ready", (data: boolean) => {
          if (!this.server) {
            this.log(`Received ready from ${playerId}, but server not running!`);
            return;
          }
          const payload: PlayerReadyPayload = { playerId, value: data };
          this.server.ioServer.emit("player-ready", payload);
        });
        socket.on("disconnect", () => {
          if (!this.server) {
            this.log(`Player ${playerId} disconnected, but server not running!`);
            return;
          }
          this.server.players = this.server.players.filter(x => x.id !== playerId);
          const playerLeft: PlayerLeftPayload = {
            playerId: playerId
          };
          socket.broadcast.emit("player-left", playerLeft);
        });
      });
    });

    const localRoomId = this.server.localRoomId;
    publishRoom(port, description, players)
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
}
