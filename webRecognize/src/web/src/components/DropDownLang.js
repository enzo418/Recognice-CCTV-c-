import React from "react";
import {Translation} from "react-i18next";
import i18n from "../i18n";

const changeLanguage = (e) => {
    i18n.changeLanguage(e.target.dataset.lang);
};

function DropDownLang() {
    return (
        <div className="dropdown" id="dropdown-language">
            <div className="dropdown-trigger">
                <button className="button" aria-haspopup="true" aria-controls="dropdown-menu">
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
                    <div className="dropdown-item is-active" data-lang="en" onClick={changeLanguage}>
                        English
                    </div>
                    <div className="dropdown-item" data-lang="es" onClick={changeLanguage}>
                        Español
                    </div>
                </div>
            </div>
        </div>
    );
}

DropDownLang.propTypes = {};

export default DropDownLang;
