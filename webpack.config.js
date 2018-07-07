const webpack = require('webpack');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const MiniCssExtractPlugin = require("mini-css-extract-plugin");
const OptimizeCssAssetsPlugin = require('optimize-css-assets-webpack-plugin');

const path = require('path');
const outputDir = path.join(__dirname, "build");
const srcDir = path.join(__dirname, "src");
const generatedSrcDir = path.join(__dirname, "lib/js/src");
const isProd = process.env.NODE_ENV === 'production';

module.exports = {
  entry: path.join(generatedSrcDir, 'Index.bs.js'),
  mode: isProd ? 'production' : 'development',
  output: {
    path: outputDir,
    publicPath: '',
    filename: 'index.js',
  },
  resolve: {
    alias: {
      style: path.join(srcDir, "style")
    }
  },
  plugins: [
    new webpack.LoaderOptionsPlugin({
      debug: !isProd
    }),
    new HtmlWebpackPlugin({
      template: path.resolve(srcDir, "index.html"),
      filename: 'index.html'
    }),
    new MiniCssExtractPlugin({
      filename: "[name].css",
      chunkFilename: "[id].css"
    }),
    new OptimizeCssAssetsPlugin({
      assetNameRegExp: /\.css$/g,
      cssProcessor: require('cssnano'),
      cssProcessorOptions: { discardComments: { removeAll: true } },
      canPrint: true
    })
  ],
  module: {
    rules: [
      { test: /\.scss$/,
        use: [
          isProd? MiniCssExtractPlugin.loader : "style-loader",
          "css-loader",
          "sass-loader"
        ]
      },
      {
        test: /\.(eot|ttf|woff|woff2)$/,
        loader: 'file-loader?name=fonts/[hash]-[name].[ext]'
      },
      {
        test: /\.(png|jpg|jpeg|gif|webp|svg)$/,
        loader: 'file-loader?name=images/[hash]-[name].[ext]'
      },
    ]
  }
};
