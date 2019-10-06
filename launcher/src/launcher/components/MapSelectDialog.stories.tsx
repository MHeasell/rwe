import React from "react";

import { storiesOf } from "@storybook/react";
import MapSelectDialog from "./MapSelectDialog";

storiesOf("MapSelectDialog", module).add("default", () => {
  const maps = [
    "Gods of War",
    "Comet Catcher",
    "Painted Desert",
    "John's Pass",
  ];
  return (
    <MapSelectDialog
      open
      maps={maps}
      selectedMap="John's Pass"
      selectedMapDetails={{
        description: "10 X 15 Follow the river up the middle, if you dare!",
        memory: "32 mb",
        numberOfPlayers: "2, 3, 4",
      }}
      onConfirm={() => {}}
      onSelect={() => {}}
      onClose={() => {}}
    />
  );
});
