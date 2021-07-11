import React from "react";
import ReactDOM from "react-dom";
import {I18nextProvider} from "react-i18next";
import i18n from "./i18n";
import App from "./App";
import {BrowserRouter as Router} from "react-router-dom";
import {withRouter} from "react-router";

// wrap the app in I18next Provider with the configuration loaded from i18n.js

const AppWithRouter = withRouter(App);

ReactDOM.render(
    <I18nextProvider i18n={i18n}>
        <Router>
            <AppWithRouter />
        </Router>
    </I18nextProvider>,
    document.getElementById("app")
);
