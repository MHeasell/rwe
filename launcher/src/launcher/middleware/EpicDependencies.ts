import { GameClientService } from "../../game-server/game-client";

import { RweBridge } from "../bridge";
import { MasterClientService } from "../../master-server/master-client";
export interface EpicDependencies {
  clientService: GameClientService;
  masterClentService: MasterClientService;
  bridgeService: RweBridge;
}
