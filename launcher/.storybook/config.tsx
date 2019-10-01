import React from "react";
import { configure, addDecorator } from "@storybook/react";
import CssBaseline from "@material-ui/core/CssBaseline";
import "../src/launcher/style.css";

const req = (require as any).context("../src", true, /\.stories\.tsx$/);

function loadStories() {
  req.keys().forEach(req);
}

configure(loadStories, module);

addDecorator(storyFn => <CssBaseline>{storyFn()}</CssBaseline>);
