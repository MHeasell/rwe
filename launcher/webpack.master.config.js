const path = require("path");
const CopyWebpackPlugin = require("copy-webpack-plugin");
const webpack = require("webpack");

module.exports = {
  mode: "development",
  entry: "./src/master-server/master-server.ts",
  target: "node",
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
                    node: "8",
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
    filename: "master-server.js",
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
    new webpack.IgnorePlugin(/^uws$/),
  ],
};
