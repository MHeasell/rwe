import * as http from "http";
import * as socketio from "socket.io";
import { GameServer, Room } from "../ws/game-server";
import * as protocol from "./protocol";
import * as yargs from "yargs";

const argv = yargs
.option("port", { alias: "p", default: 5000 })
.argv;

const port = argv.port;

const server = http.createServer().listen(port);
const io = socketio(server, { serveClient: false });

function roomToEntry(x: Room): protocol.GetGamesReponseEntry {
  return {
    description: x.description,
    players: x.players.length,
    max_players: x.maxPlayers,
  };
}

function log(msg: string) {
  console.log(`master server: ${msg}`);
}

const masterNamespace = io.of("/master");
const roomsNamespace = io.of("/rooms");

const gameServer = new GameServer(roomsNamespace);

gameServer.gameUpdated.subscribe(([roomId, room]) => {
  const payload: protocol.GameUpdatedEventPayload = {
    game_id: roomId,
    game: roomToEntry(room),
  };
  masterNamespace.emit(protocol.GameUpdatedEvent, payload);
});

gameServer.gameDeleted.subscribe(id => {
  const payload: protocol.GameDeletedEventPayload = {
    game_id: id,
  };
  masterNamespace.emit(protocol.GameDeletedEvent, payload);
});

masterNamespace.on("connection", socket => {
  const addr = socket.handshake.address;
  log(`Received connection from ${addr}`);

  {
    const gamesList = Array.from(gameServer.getAllRooms())
    .map(([roomId, room]) => {
      return { id: roomId, game: roomToEntry(room) };
    });

    const payload: protocol.GetGamesResponsePayload = { games: gamesList };
    socket.emit(protocol.GetGamesResponse, payload);
  }

  socket.on(protocol.GetGames, () => {
    const gamesList = Array.from(gameServer.getAllRooms())
    .map(([roomId, room]) => {
      return { id: roomId, game: roomToEntry(room) };
    });

    const payload: protocol.GetGamesResponsePayload = { games: gamesList };
    socket.emit(protocol.GetGamesResponse, payload);
  });

  socket.on(protocol.CreateGameRequest, (data: protocol.CreateGameRequestPayload) => {
    const gameInfo = gameServer.createRoom(data.description, data.max_players);

    const payload: protocol.CreateGameResponsePayload = {
      game_id: gameInfo.gameId,
      admin_key: gameInfo.adminKey,
    };
    socket.emit(protocol.CreateGameResponse, payload);

    const eventPayload: protocol.GameCreatedEventPayload = {
      game_id: gameInfo.gameId,
      game: {
        description: data.description,
        players: 0,
        max_players: data.max_players,
      },
    };
    masterNamespace.emit(protocol.GameCreatedEvent, eventPayload);
  });

  socket.on("disconnect", () => {
    log(`Client from ${addr} disconnected`);
  });

  socket.on("error", (error: Error) => {
    log(`Error from ${addr}: ${error}`);
  });
});

log("master server started");
