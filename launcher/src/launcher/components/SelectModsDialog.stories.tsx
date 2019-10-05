import React from "react";

import { storiesOf } from "@storybook/react";
import { SelectModsDialog } from "./SelectModsDialog";

storiesOf("SelectModsDialog", module).add("default", () => {
  const installedMods = ["ta", "tacc", "ta31", "taesc"];
  const activeMods = ["ta", "tacc"];
  return (
    <SelectModsDialog
      title="Select Mods"
      items={installedMods}
      initiallyActiveItems={activeMods}
      onSubmit={() => {}}
      onCancel={() => {}}
    />
  );
});
