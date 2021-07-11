import React from "react";
import PropTypes from "prop-types";

import {Translation} from "react-i18next";
import DropDownLang from "./DropDownLang";
import {Link} from "react-router-dom";

class HomeNavBar extends React.Component {
    constructor(props) {
        super(props);
    }

    render() {
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
                                {(t) => (
                                    <h1 id="button-state-recognizer">
                                        {this.props.recognize.running
                                            ? t("Recognizer is running")
                                            : t("Recognizer is not running")}
                                    </h1>
                                )}
                            </Translation>
                        </div>

                        <div className="navbar-item">
                            <Translation>
                                {(t) => (
                                    <button
                                        className="button is-success"
                                        id="button-toggle-recognizer"
                                        onClick={() => this.props.recognize.toggle()}>
                                        {this.props.recognize.running ? t("Stop recognizer") : t("Start Recognizer")}
                                    </button>
                                )}
                            </Translation>
                        </div>

                        {/* {this.props.recognize.configuration.file !== "" && ( */}
                        <div className="navbar-item">
                            {/* TODO: Set a different bg if it's the current page  */}
                            <Translation>
                                {(t) => <Link to={this.props.pages.configurations.path}>{t("configurations")}</Link>}
                            </Translation>
                        </div>
                        {/* )} */}

                        <div className="navbar-item">
                            <Translation>
                                {(t) => <Link to={this.props.pages.notifications.path}>{t("notifications")}</Link>}
                            </Translation>
                        </div>

                        <div className="navbar-item">
                            <DropDownLang></DropDownLang>
                        </div>
                    </div>
                </div>
            </nav>
        );
    }
}

HomeNavBar.propTypes = {
    pages: PropTypes.object.isRequired,
    recognize: PropTypes.object.isRequired,
    toggleRecognize: PropTypes.func.isRequired,
};

export default HomeNavBar;
