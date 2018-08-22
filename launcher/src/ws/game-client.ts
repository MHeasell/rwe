import * as socketioClient from "socket.io-client";
import * as protocol from "./protocol";
import { Observable, Subject } from "rxjs";

export class GameClientService {
  private client: SocketIOClient.Socket | undefined;

  private readonly _onDisconnect: Subject<void> = new Subject();
  private readonly _onHandshakeResponse: Subject<protocol.HandshakeResponsePayload> = new Subject();
  private readonly _onPlayerJoined: Subject<protocol.PlayerJoinedPayload> = new Subject();
  private readonly _onPlayerLeft: Subject<protocol.PlayerLeftPayload> = new Subject();
  private readonly _onPlayerChatMessage: Subject<protocol.PlayerChatMessagePayload> = new Subject();
  private readonly _onPlayerReady: Subject<protocol.PlayerReadyPayload> = new Subject();
  private readonly _onStartGame: Subject<void> = new Subject();

  get onDisconnect(): Observable<void> { return this._onDisconnect; }
  get onHandshakeResponse(): Observable<protocol.HandshakeResponsePayload> { return this._onHandshakeResponse; }
  get onPlayerJoined(): Observable<protocol.PlayerJoinedPayload> { return this._onPlayerJoined; }
  get onPlayerLeft(): Observable<protocol.PlayerLeftPayload> { return this._onPlayerLeft; }
  get onPlayerChatMessage(): Observable<protocol.PlayerChatMessagePayload> { return this._onPlayerChatMessage; }
  get onPlayerReady(): Observable<protocol.PlayerReadyPayload> { return this._onPlayerReady; }
  get onStartGame(): Observable<void> { return this._onStartGame; }

  connectToServer(connectionString: string, roomId: number, playerName: string) {
    this.client = socketioClient(connectionString);

    this.client.on("connect_error", (error: any) => {
      console.log(error);
    });

    this.client.on("disconnect", () => {
      this.disconnect();
      this._onDisconnect.next();
    });

    const handshakePayload: protocol.HandshakePayload = {
      roomId: roomId,
      name: playerName,
    };
    this.client.emit(protocol.Handshake, handshakePayload);

    this.client.on(protocol.HandshakeResponse, (data: protocol.HandshakeResponsePayload) => {
      this._onHandshakeResponse.next(data);
    });

    this.client.on(protocol.PlayerJoined, (data: protocol.PlayerJoinedPayload) => {
      this._onPlayerJoined.next(data);
    });

    this.client.on(protocol.PlayerLeft, (data: protocol.PlayerLeftPayload) => {
      this._onPlayerLeft.next(data);
    });

    this.client.on(protocol.PlayerChatMessage, (data: protocol.PlayerChatMessagePayload) => {
      this._onPlayerChatMessage.next(data);
    });

    this.client.on(protocol.PlayerReady, (data: protocol.PlayerReadyPayload) => {
      this._onPlayerReady.next(data);
    });

    this.client.on(protocol.StartGame, () => {
      this._onStartGame.next();
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
