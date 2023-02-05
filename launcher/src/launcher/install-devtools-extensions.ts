import installExtension, { REDUX_DEVTOOLS } from "electron-devtools-installer";

export function installExtensions(): Promise<any> {
  return installExtension(REDUX_DEVTOOLS)
    .then((name: string) => {
      console.log(`Installed ${name}`);
    })
    .catch((err: string) => {
      console.log("An error occurred: ", err);
    });
}
