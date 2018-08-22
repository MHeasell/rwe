function postJson(url: string, data: {}) {
  return fetch(url, {
    method: "POST",
    headers: {
      "Content-Type": "application/json; charset=utf-8",
    },
    body: JSON.stringify(data),
  });
}

function deleteJson(url: string, data: {}) {
  return fetch(url, {
    method: "DELETE",
    headers: {
      "Content-Type": "application/json; charset=utf-8",
    },
    body: JSON.stringify(data),
  });
}

export interface CreateRoomRequest {
  port: number;
  local_room_id: number;
  description: string;
  number_of_players: number;
  max_players: number;
}

export interface CreateRoomResponse {
  id: number;
  key: string;
}

export function createRoom(request: CreateRoomRequest) {
  return postJson("http://localhost:5000/rooms", request)
  .then(r => r.json())
  .then(r => r as CreateRoomResponse);
}

export interface KeepAliveRoomRequest {
  key: string;
  number_of_players: number;
}

export interface KeepAliveRoomResponse {
  result: "success";
}

export function keepAliveRoom(roomId: number, request: KeepAliveRoomRequest) {
  return postJson(`http://localhost:5000/rooms/${roomId}/keep-alive`, request)
  .then(r => r.json())
  .then(r => r as KeepAliveRoomResponse);
}

export interface DeleteRoomRequest {
  key: string;
}

export interface DeleteRoomResponse { }

export function deleteRoom(roomId: number, request: DeleteRoomRequest) {
  return deleteJson(`http://localhost:5000/rooms/${roomId}`, request)
  .then(r => r.json())
  .then(r => r as DeleteRoomResponse);
}

export interface GetRoomsResponseRoom {
  local_room_id: number;
  description: string;
  host: string;
  number_of_players: number;
  max_players: number;
  port: number;
}

export interface GetRoomsResponseRoomEntry {
  id: number;
  room: GetRoomsResponseRoom;
}

export interface GetRoomsResponse {
  rooms: GetRoomsResponseRoomEntry[];
}

export function getRooms() {
  return fetch("http://localhost:5000/rooms")
  .then(r => r.json())
  .then(r => r as GetRoomsResponse);
}

// export function getRooms(): Promise<GetRoomsResponse> {
//   const dummyResponse: GetRoomsResponse = {
//     rooms: [
//       {
//         id: 1,
//         room: {
//           description: "my cool game",
//           host: "1.2.3.4",
//           port: 1234,
//           number_of_players: 2,
//         },
//       },
//       {
//         id: 2,
//         room: {
//           description: "some other game",
//           host: "1.2.3.4",
//           port: 1234,
//           number_of_players: 6,
//         },
//       },
//       {
//         id: 3,
//         room: {
//           description: "full game",
//           host: "1.2.3.4",
//           port: 1234,
//           number_of_players: 10,
//         },
//       },
//     ],
//   };

//   return new Promise(resolve => { resolve(dummyResponse); });
// }
