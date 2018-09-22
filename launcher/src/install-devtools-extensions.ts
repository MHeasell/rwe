import installExtension, { REACT_DEVELOPER_TOOLS, REDUX_DEVTOOLS } from "electron-devtools-installer";

export function installExtensions(): Promise<any> {
  return installExtension(REACT_DEVELOPER_TOOLS)
    .then((name: string) => {
      console.log(`Installed ${name}`);
      return installExtension(REDUX_DEVTOOLS);
    })
    .then((name: string) => {
      console.log(`Installed ${name}`);
    })
    .catch((err: string) => {
      console.log("An error occurred: ", err);
    });
}
