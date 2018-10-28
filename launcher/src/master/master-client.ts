import { Observable, Subject } from "rxjs";
import * as socketioClient from "socket.io-client";
import * as protocol from "./protocol";

export class MasterClientService {
  private client: SocketIOClient.Socket | undefined;

  private readonly _onGetGamesReponse = new Subject<protocol.GetGamesResponsePayload>();
  private readonly _onCreateGameReponse = new Subject<protocol.CreateGameResponsePayload>();
  private readonly _onConnect = new Subject<void>();
  private readonly _onDisconnect = new Subject<void>();
  private readonly _onGameCreated = new Subject<protocol.GameCreatedEventPayload>();
  private readonly _onGameUpdated = new Subject<protocol.GameUpdatedEventPayload>();
  private readonly _onGameDeleted = new Subject<protocol.GameDeletedEventPayload>();

  get onGetGamesResponse(): Observable<protocol.GetGamesResponsePayload> { return this._onGetGamesReponse; }
  get onCreateGameResponse(): Observable<protocol.CreateGameResponsePayload> { return this._onCreateGameReponse; }
  get onConnect(): Observable<void> { return this._onConnect; }
  get onDisconnect(): Observable<void> { return this._onDisconnect; }
  get onGameCreated(): Observable<protocol.GameCreatedEventPayload> { return this._onGameCreated; }
  get onGameUpdated(): Observable<protocol.GameUpdatedEventPayload> { return this._onGameUpdated; }
  get onGameDeleted(): Observable<protocol.GameDeletedEventPayload> { return this._onGameDeleted; }

  connectToServer(connectionString: string) {
    this.client = socketioClient(connectionString);

    this.client.on("connect", () => {
      this.log("connected");
      this._onConnect.next();
    });

    this.client.on("disconnect", () => {
      this.log("disconnected");
      this._onDisconnect.next();
    });

    this.client.on("connect_error", (e: Error) => { this.log(`connection error: ${e}`); });
    this.client.on("connect_timeout", () => { this.log("connection timeout"); });
    this.client.on("reconnect", (attempt: number) => { this.log(`successful reconnect attempt ${attempt}`); });
    this.client.on("reconnect_attempt", () => { this.log("reconnect attempt"); });
    this.client.on("reconnecting", (attempt: number) => { this.log(`reconnecting attempt ${attempt}`); });
    this.client.on("reconnect_failed", () => { this.log("failed to reconnect and gave up"); });

    this.client.on(protocol.GetGamesResponse, (data: protocol.GetGamesResponsePayload) => {
      this._onGetGamesReponse.next(data);
    });

    this.client.on(protocol.CreateGameResponse, (data: protocol.CreateGameResponsePayload) => {
      this._onCreateGameReponse.next(data);
    });

    this.client.on(protocol.GameCreatedEvent, (data: protocol.GameCreatedEventPayload) => {
      this._onGameCreated.next(data);
    });

    this.client.on(protocol.GameUpdatedEvent, (data: protocol.GameUpdatedEventPayload) => {
      this._onGameUpdated.next(data);
    });

    this.client.on(protocol.GameDeletedEvent, (data: protocol.GameDeletedEventPayload) => {
      this._onGameDeleted.next(data);
    });
  }

  requestCreateGame(description: string, maxPlayers: number) {
    if (!this.client) { return; }
    const payload: protocol.CreateGameRequestPayload = {
      description,
      max_players: maxPlayers,
    };
    this.client.emit(protocol.CreateGameRequest, payload);
  }

  getGames() {
    if (!this.client) { return; }
    this.client.emit(protocol.GetGames);
  }

  private log(msg: string) {
    console.log(`master client: ${msg}`);
  }
}
