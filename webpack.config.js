const path = require('path');
const outputDir = path.join(__dirname, "build/");
const srcDir = path.join(__dirname, "lib/js/src");

const isProd = process.env.NODE_ENV === 'production';

module.exports = {
  entry: path.join(srcDir, 'Index.bs.js'),
  mode: isProd ? 'production' : 'development',
  output: {
    path: outputDir,
    publicPath: outputDir,
    filename: 'Index.js',
  },
};
