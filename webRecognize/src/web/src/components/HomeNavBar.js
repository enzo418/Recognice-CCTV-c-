import React from "react";
import PropTypes from "prop-types";

import {withTranslation} from "react-i18next";
import DropDownLang from "./DropDownLang";
import {Link} from "react-router-dom";

class HomeNavBar extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            showNavBar: false,
        };
    }

    toggleNavBar() {
        this.setState((prev) => {
            prev.showNavBar = !prev.showNavBar;
            return prev;
        });
    }

    render() {
        const {t} = this.props;
        return (
            <nav
                className={"navbar is-black" + (this.state.showNavBar ? " is-active" : "")}
                role="navigation"
                aria-label="main navigation">
                <div className="navbar-brand">
                    <a
                        role="button"
                        className="navbar-burger"
                        aria-label="menu"
                        aria-expanded="false"
                        data-target="navbar-status"
                        onClick={() => this.toggleNavBar()}>
                        <span aria-hidden="true"></span>
                        <span aria-hidden="true"></span>
                        <span aria-hidden="true"></span>
                    </a>
                </div>

                <div id="navbar-status" className="navbar-menu">
                    <div className="navbar-start">
                        <div className="navbar-item">
                            <h1 id="button-state-recognizer">
                                {this.props.recognize.running
                                    ? t("recognizer is running")
                                    : t("recognizer is not running")}
                            </h1>
                        </div>

                        <div className="navbar-item">
                            <button
                                className={`button ${!this.props.recognize.running ? "is-success" : "is-danger"}`}
                                id="button-toggle-recognizer"
                                onClick={() => this.props.recognize.toggle()}>
                                {this.props.recognize.running && t("stop Recognizer")}
                                {!this.props.recognize.running && t("start Recognizer")}
                            </button>
                        </div>

                        {this.props.recognize.configuration.file !== "" && (
                            <div className="navbar-item">
                                {/* TODO: Set a different bg if it's the current page  */}

                                <Link to={this.props.pages.configurations.path}>{t("configurations")}</Link>
                            </div>
                        )}

                        <div className="navbar-item">
                            <Link to={this.props.pages.notifications.path}>{t("notifications")}</Link>
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
    t: PropTypes.func,
};

export default withTranslation()(HomeNavBar);
