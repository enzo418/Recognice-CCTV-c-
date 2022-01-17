import React from "react";
import ReactDOM from "react-dom";
import {I18nextProvider} from "react-i18next";
import i18n from "./i18n";
import App from "./App";
import {BrowserRouter as Router} from "react-router-dom";
import {withRouter} from "react-router";
import "@fortawesome/fontawesome-free/js/all.js";

// wrap the app in I18next Provider with the configuration loaded from i18n.js
const AppWithRouter = withRouter(App);

ReactDOM.render(
    <React.StrictMode><App></App></React.StrictMode>,
    document.getElementById("app")
);
