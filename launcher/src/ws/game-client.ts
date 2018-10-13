import * as socketioClient from "socket.io-client";
import * as protocol from "./protocol";
import { Observable, Subject } from "rxjs";
import { PlayerSide } from "../state";

export class GameClientService {
  private client: SocketIOClient.Socket | undefined;

  private readonly _onDisconnect = new Subject<void>();
  private readonly _onHandshakeResponse = new Subject<protocol.HandshakeResponsePayload>();
  private readonly _onPlayerJoined = new Subject<protocol.PlayerJoinedPayload>();
  private readonly _onPlayerLeft = new Subject<protocol.PlayerLeftPayload>();
  private readonly _onPlayerChatMessage = new Subject<protocol.PlayerChatMessagePayload>();
  private readonly _onPlayerChangedSide = new Subject<protocol.PlayerChangedSidePayload>();
  private readonly _onPlayerChangedTeam = new Subject<protocol.PlayerChangedTeamPayload>();
  private readonly _onPlayerChangedColor = new Subject<protocol.PlayerChangedColorPayload>();
  private readonly _onSlotOpened = new Subject<protocol.SlotOpenedPayload>();
  private readonly _onSlotClosed = new Subject<protocol.SlotClosedPayload>();
  private readonly _onPlayerReady = new Subject<protocol.PlayerReadyPayload>();
  private readonly _onStartGame = new Subject<void>();

  get onDisconnect(): Observable<void> { return this._onDisconnect; }
  get onHandshakeResponse(): Observable<protocol.HandshakeResponsePayload> { return this._onHandshakeResponse; }
  get onPlayerJoined(): Observable<protocol.PlayerJoinedPayload> { return this._onPlayerJoined; }
  get onPlayerLeft(): Observable<protocol.PlayerLeftPayload> { return this._onPlayerLeft; }
  get onPlayerChatMessage(): Observable<protocol.PlayerChatMessagePayload> { return this._onPlayerChatMessage; }
  get onPlayerChangedSide(): Observable<protocol.PlayerChangedSidePayload> { return this._onPlayerChangedSide; }
  get onPlayerChangedTeam(): Observable<protocol.PlayerChangedTeamPayload> { return this._onPlayerChangedTeam; }
  get onPlayerChangedColor(): Observable<protocol.PlayerChangedColorPayload> { return this._onPlayerChangedColor; }
  get onSlotOpened(): Observable<protocol.SlotOpenedPayload> { return this._onSlotOpened; }
  get onSlotClosed(): Observable<protocol.SlotClosedPayload> { return this._onSlotClosed; }
  get onPlayerReady(): Observable<protocol.PlayerReadyPayload> { return this._onPlayerReady; }
  get onStartGame(): Observable<void> { return this._onStartGame; }

  connectToServer(connectionString: string, gameId: number, playerName: string, adminKey?: string) {
    this.client = socketioClient(connectionString);

    this.client.on("connect", () => { this.log("Connected"); });
    this.client.on("connect_error", (error: Error) => { this.log(`Connection error: ${error}`); });
    this.client.on("connect_timeout", () => { this.log("Connection timeout"); });
    this.client.on("error", (error: Error) => { this.log(`Error: ${error}`); });

    this.client.on("reconnect", (attempt: number) => { this.log(`successful reconnect attempt ${attempt}`); });
    this.client.on("reconnect_attempt", () => { this.log("reconnect attempt"); });
    this.client.on("reconnecting", (attempt: number) => { this.log(`reconnecting attempt ${attempt}`); });
    this.client.on("reconnect_failed", () => { this.log("failed to reconnect and gave up"); });

    this.client.on("disconnect", () => {
      this.log("Disconnected");
      this.disconnect();
    });

    const handshakePayload: protocol.HandshakePayload = {
      gameId,
      name: playerName,
    };
    if (adminKey !== undefined) { handshakePayload.adminKey = adminKey; }
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

    this.client.on(protocol.PlayerChangedSide, (data: protocol.PlayerChangedSidePayload) => {
      this._onPlayerChangedSide.next(data);
    });

    this.client.on(protocol.PlayerChangedTeam, (data: protocol.PlayerChangedTeamPayload) => {
      this._onPlayerChangedTeam.next(data);
    });

    this.client.on(protocol.PlayerChangedColor, (data: protocol.PlayerChangedColorPayload) => {
      this._onPlayerChangedColor.next(data);
    });

    this.client.on(protocol.SlotOpened, (data: protocol.SlotOpenedPayload) => {
      this._onSlotOpened.next(data);
    });

    this.client.on(protocol.SlotClosed, (data: protocol.SlotClosedPayload) => {
      this._onSlotClosed.next(data);
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
    this._onDisconnect.next();
  }

  sendChatMessage(message: string) {
    if (!this.client) { return; }
    const payload: protocol.ChatMessagePayload = message;
    this.client.emit(protocol.ChatMessage, payload);
  }

  changeSide(side: PlayerSide) {
    if (!this.client) { return; }
    const payload: protocol.ChangeSidePayload = { side };
    this.client.emit(protocol.ChangeSide, payload);
  }

  changeTeam(team?: number) {
    if (!this.client) { return; }
    const payload: protocol.ChangeTeamPayload = { team };
    this.client.emit(protocol.ChangeTeam, payload);
  }

  changeColor(color: number) {
    if (!this.client) { return; }
    const payload: protocol.ChangeColorPayload = { color };
    this.client.emit(protocol.ChangeColor, payload);
  }

  openSlot(slotId: number) {
    if (!this.client) { return; }
    const payload: protocol.OpenSlotPayload = { slotId };
    this.client.emit(protocol.OpenSlot, payload);
  }

  closeSlot(slotId: number) {
    if (!this.client) { return; }
    const payload: protocol.CloseSlotPayload = { slotId };
    this.client.emit(protocol.CloseSlot, payload);
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

  private log(msg: string) {
    console.log(`Game client: ${msg}`);
  }
}
