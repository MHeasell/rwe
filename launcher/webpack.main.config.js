const path = require("path");
const CopyWebpackPlugin = require("copy-webpack-plugin");

module.exports = {
  mode: "development",
  entry: "./src/launcher/main.ts",
  target: "electron-main",
  devtool: "source-map",
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: {
          loader: "babel-loader",
          options: {
            presets: [
              [
                "@babel/env",
                {
                  targets: {
                    electron: "6.0.9",
                    node: "12.4.0",
                  },
                },
              ],
              "@babel/typescript",
            ],
            plugins: ["@babel/proposal-class-properties"],
          },
        },
        exclude: /node_modules/,
      },
    ],
  },
  resolve: {
    extensions: [".ts", ".tsx", ".js", ".json"],
  },
  output: {
    filename: "main.js",
    path: path.resolve(__dirname, "dist"),
  },
  node: {
    __dirname: false,
    __filename: false,
  },
  plugins: [
    new CopyWebpackPlugin([
      {
        from: "package.json",
        to: "package.json",
      },
    ]),
  ],
};
