import React, {useState} from "react";
import {Translation} from "react-i18next";
import i18n from "../i18n";

const changeLanguage = (e, setDisplayed) => {
    setDisplayed(false);
    i18n.changeLanguage(e.target.dataset.lang);
};

function DropDownLang() {
    const [displayed, setDisplayed] = useState(false);
    return (
        <div className={"dropdown" + (displayed ? " is-active" : "")}>
            <div className="dropdown-trigger">
                <button
                    className="button"
                    aria-haspopup="true"
                    aria-controls="dropdown-menu"
                    onClick={() => setDisplayed(!displayed)}>
                    <span className="icon is-small">
                        <i className="fas fa-globe" aria-hidden="true"></i>
                    </span>
                    <Translation>{(t) => <span>{t("Language")}</span>}</Translation>
                    <span className="icon is-small">
                        <i className="fas fa-angle-down" aria-hidden="true"></i>
                    </span>
                </button>
            </div>
            <div className="dropdown-menu" id="dropdown-menu" role="menu">
                <div className="dropdown-content">
                    <div
                        className="dropdown-item is-active"
                        data-lang="en"
                        onClick={(e) => changeLanguage(e, setDisplayed)}>
                        English
                    </div>
                    <div className="dropdown-item" data-lang="es" onClick={(e) => changeLanguage(e, setDisplayed)}>
                        Español
                    </div>
                </div>
            </div>
        </div>
    );
}

DropDownLang.propTypes = {};

export default DropDownLang;
