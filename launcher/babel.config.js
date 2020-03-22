module.exports = {
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
};
