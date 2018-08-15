import * as socketioClient from "socket.io-client";
import { MiddlewareAPI, Dispatch } from "redux";
import { State } from "../state";
import { receiveChatMessage, receiveHandshakeResponse, receivePlayerJoined, receivePlayerLeft, disconnectGame, receivePlayerReady, receiveStartGame } from "../actions";
import * as protocol from "./protocol";

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

    const handshakePayload: protocol.HandshakePayload = {
      name: playerName,
    };
    this.client.emit(protocol.Handshake, handshakePayload);

    this.client.on(protocol.HandshakeResponse, (data: protocol.HandshakeResponsePayload) => {
      this.store.dispatch(receiveHandshakeResponse(data));
    });

    this.client.on(protocol.PlayerJoined, (data: protocol.PlayerJoinedPayload) => {
      this.store.dispatch(receivePlayerJoined(data));
    });

    this.client.on(protocol.PlayerLeft, (data: protocol.PlayerLeftPayload) => {
      this.store.dispatch(receivePlayerLeft(data));
    });

    this.client.on(protocol.PlayerChatMessage, (data: protocol.PlayerChatMessagePayload) => {
      this.store.dispatch(receiveChatMessage(data));
    });

    this.client.on(protocol.PlayerReady, (data: protocol.PlayerReadyPayload) => {
      this.store.dispatch(receivePlayerReady(data));
    });

    this.client.on(protocol.StartGame, () => {
      this.store.dispatch(receiveStartGame());
    });
  }

  disconnect() {
    if (!this.client) { return; }
    this.client.close();
    this.client = undefined;
  }

  sendChatMessage(message: string) {
    if (!this.client) { return; }
    const payload: protocol.ChatMessagePayload = message;
    this.client.emit(protocol.ChatMessage, payload);
  }

  setReadyState(value: boolean) {
    if (!this.client) { return; }
    const payload: protocol.ReadyPayload = value;
    this.client.emit(protocol.Ready, payload);
  }

  requestStartGame() {
    if (!this.client) { return; }
    this.client.emit(protocol.RequestStartGame);
  }
}
