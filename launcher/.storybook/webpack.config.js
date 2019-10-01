module.exports = ({ config }) => {
  config.module.rules.push({
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
        plugins: ["@babel/proposal-class-properties"],
      },
    },
  });
  config.resolve.extensions.push(".ts", ".tsx");
  return config;
};
