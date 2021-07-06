const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

const output_dir = '../../build/web/';

module.exports = {
    devServer: {
        contentBase: path.resolve(output_dir),
        compress: true,
        port: 8500,
    },
    entry: './src/app.js',
    output: {
        filename: 'main.js',
        path: path.resolve(output_dir),
        clean: true
    },
    plugins: [
        new HtmlWebpackPlugin({
            template: './src/index.html',
            favicon: 'assets/favicon.svg'
        }),
    ],
    module: {
        rules: [
            {
            test: /\.css$/i,
            use: ['style-loader', 'css-loader'],
            },
        ],
    },
    resolve: {
        modules: [path.resolve(__dirname, 'src/modules'), 'node_modules'],
    },
    watchOptions: {
        ignored: '**/node_modules',
    },
};