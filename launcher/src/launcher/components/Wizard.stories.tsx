import React from "react";
import { Wizard } from "./Wizard";

import { storiesOf } from "@storybook/react";

storiesOf("Wizard", module)
  .add("welcome", () => {
    return <Wizard state="welcome" onNext={() => {}} onClose={() => {}} />;
  })
  .add("working", () => (
    <Wizard state="working" onNext={() => {}} onClose={() => {}} />
  ))
  .add("success", () => (
    <Wizard state="success" onNext={() => {}} onClose={() => {}} />
  ))
  .add("fail", () => (
    <Wizard state="fail" onNext={() => {}} onClose={() => {}} />
  ));
