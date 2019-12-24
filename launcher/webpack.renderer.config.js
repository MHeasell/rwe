const path = require("path");
const CopyWebpackPlugin = require("copy-webpack-plugin");
const webpack = require("webpack");

module.exports = {
  mode: "development",
  entry: ["react-hot-loader/patch", "./src/launcher/renderer.tsx"],
  target: "electron-renderer",
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
              "@babel/react",
              "@babel/typescript",
            ],
            plugins: [
              "react-hot-loader/babel",
              "@babel/proposal-class-properties",
            ],
          },
        },
        exclude: /node_modules/,
      },
      {
        test: /\.css$/,
        use: ["style-loader", "css-loader"],
      },
      {
        test: /\.ttf$/,
        use: [
          {
            loader: "file-loader",
          },
        ],
      },
      {
        test: /\.node$/,
        use: "node-loader",
      },
    ],
  },
  resolve: {
    extensions: [".ts", ".tsx", ".js", ".json"],
    alias: {
      "react-dom": "@hot-loader/react-dom",
    },
  },
  output: {
    filename: "renderer.js",
    path: path.resolve(__dirname, "dist"),
  },
  plugins: [
    new CopyWebpackPlugin([
      {
        from: "index.html",
        to: "index.html",
      },
    ]),
    new webpack.NamedModulesPlugin(),
    new webpack.HotModuleReplacementPlugin(),
    new webpack.IgnorePlugin(/^uws$/),
  ],

  devServer: {
    contentBase: "./dist",
    port: 8080,
    hot: true,
  },
};
