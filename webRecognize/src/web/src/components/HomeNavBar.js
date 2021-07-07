import React from "react";
import PropTypes from "prop-types";

import {Translation} from "react-i18next";
import DropDownLang from "./DropDownLangDrop";

const startRecognizer = () => {
    throw "Method not implemented";
};

const toggleConfigurationNotificationPage = () => {
    throw "Method not implemented";
};

function HomeNavBar() {
    return (
        <nav className="navbar is-black" role="navigation" aria-label="main navigation">
            <div className="navbar-brand">
                <a
                    role="button"
                    className="navbar-burger"
                    aria-label="menu"
                    aria-expanded="false"
                    data-target="navbar-status">
                    <span aria-hidden="true"></span>
                    <span aria-hidden="true"></span>
                    <span aria-hidden="true"></span>
                </a>
            </div>

            <div id="navbar-status" className="navbar-menu">
                <div className="navbar-start">
                    <div className="navbar-item">
                        <Translation>
                            {(t) => <h1 id="button-state-recognizer">{t("Recognizer is not running")}</h1>}
                        </Translation>
                    </div>

                    <div className="navbar-item">
                        <Translation>
                            {(t) => (
                                <button
                                    className="button is-success"
                                    id="button-toggle-recognizer"
                                    onClick={startRecognizer}>
                                    {t("Start Recognizer")}
                                </button>
                            )}
                        </Translation>
                    </div>

                    <div className="navbar-item">
                        <Translation>
                            {(t) => (
                                <h1 id="button-current-page" onClick={toggleConfigurationNotificationPage}>
                                    {t("Show notifications page")}
                                </h1>
                            )}
                        </Translation>
                    </div>

                    <div className="navbar-item">{DropDownLang}</div>
                </div>
            </div>
        </nav>
    );
}

HomeNavBar.propTypes = {
    group_id: PropTypes.string.isRequired,
};
