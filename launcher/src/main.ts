import { app, BrowserWindow, BrowserWindowConstructorOptions } from "electron";
import * as path from "path";
import { installExtensions } from "./install-devtools-extensions";

const development = !!process.env["RWE_LAUNCHER_IS_DEV"];
console.log(`Running in ${development ? "development" : "production"} mode`);

// We intentionally load "remote" content during development
// because we use the webpack-dev-server
// to provide hot-reload features.
if (development) {
  process.env["ELECTRON_DISABLE_SECURITY_WARNINGS"] = "1";
  if (!process.env["RWE_MASTER_SERVER"]) {
    process.env["RWE_MASTER_SERVER"] = "http://localhost:5000";
  }
}

if (!process.env["RWE_HOME"]) {
  // When packaged, getAppPath() will return path/to/launcher/resources/app.asar.
  // The rwe binary will be three levels up from here.
  process.env["RWE_HOME"] = path.resolve(app.getAppPath(), "..", "..", "..");
}

let mainWindow: Electron.BrowserWindow | null;

function createWindow() {
  // Create the browser window.
  const windowOptions: BrowserWindowConstructorOptions = {
    height: 600,
    width: 800,
    webPreferences: {
      // Enabling node integration is safe
      // because we only ever run local, trusted code.
      nodeIntegration: true,
    },
  };
  // disable web security in development, to permit accessing local files
  // even though the page is served from a remote URL
  if (development) {
    windowOptions.webPreferences!.webSecurity = false;
  }
  mainWindow = new BrowserWindow(windowOptions);

  // and load the index.html of the app.
  if (development) {
    mainWindow.loadURL("http://localhost:8080/index.html");
  }
  else {
    mainWindow.loadFile("index.html");
  }

  // Open the DevTools.
  if (development) {
    mainWindow.webContents.openDevTools();
  }

  // Emitted when the window is closed.
  mainWindow.on("closed", () => {
    // Dereference the window object, usually you would store windows
    // in an array if your app supports multi windows, this is the time
    // when you should delete the corresponding element.
    mainWindow = null;
  });
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on("ready", () => installExtensions().then(createWindow));

// Quit when all windows are closed.
app.on("window-all-closed", () => {
  // On OS X it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  if (process.platform !== "darwin") {
    app.quit();
  }
});

app.on("activate", () => {
  // On OS X it"s common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (mainWindow === null) {
    createWindow();
  }
});

// In this file you can include the rest of your app"s specific main process
// code. You can also put them in separate files and require them here.
