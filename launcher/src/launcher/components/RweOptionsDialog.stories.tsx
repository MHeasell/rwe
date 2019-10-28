import React from "react";

import { storiesOf } from "@storybook/react";
import { RweOptionsDialog } from "./RweOptionsDialog";

storiesOf("RweOptionsDialog", module).add("default", () => {
  const items = [
    { width: 640, height: 480 },
    { width: 800, height: 600 },
    { width: 1024, height: 768 },
  ];

  return (
    <RweOptionsDialog
      videoModes={items}
      onSubmit={() => {}}
      onCancel={() => {}}
    />
  );
});
