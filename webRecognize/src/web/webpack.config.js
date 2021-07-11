const path = require("path");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");

const output_dir = "../../build/web/";

module.exports = {
    devServer: {
        contentBase: path.resolve(output_dir),
        compress: true,
        port: 8000,
    },
    entry: "./src/index.js",
    output: {
        filename: "main.js",
        path: path.resolve(output_dir),
        clean: true,
    },
    plugins: [
        new HtmlWebpackPlugin({
            template: "./src/index.html",
            favicon: "assets/favicon.svg",
        }),
        new MiniCssExtractPlugin({
            filename: "css/styles.css",
        }),
    ],
    module: {
        rules: [
            {
                test: /\.css$/i,
                use: ["style-loader", "css-loader"],
            },
            {
                test: /\.(js|jsx)$/,
                exclude: /node_modules/,
                use: ["babel-loader"],
            },
            {
                test: /\.scss$/,
                use: [
                    MiniCssExtractPlugin.loader,
                    {
                        loader: "css-loader",
                    },
                    {
                        loader: "sass-loader",
                        options: {
                            sourceMap: true,
                            // options...
                        },
                    },
                ],
            },
        ],
    },
    resolve: {
        modules: [path.resolve(__dirname, "src/modules"), path.resolve(__dirname, "src/components"), "node_modules"],
        extensions: ["*", ".js", ".jsx"],
    },
    watchOptions: {
        ignored: /node_modules/,
    },
};
