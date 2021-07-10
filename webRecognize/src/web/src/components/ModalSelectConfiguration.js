import React from "react";
import {Translation} from "react-i18next";

function ModalSelectConfiguration() {
    function showFilesDropDown() {
        throw "Method not implemented";
    }

    function goToNotificationsPage() {
        throw "Method not implemented";
    }

    function copyFileAndSelectIt() {
        throw "Method not implemented";
    }

    function selectFile() {
        throw "Method not implemented";
    }

    return (
        <div className="modal" id="modal-file">
            <div className="modal-background"></div>
            <div className="modal-content container is-fullhd">
                <article className="message is-primary">
                    <div className="message-header">
                        <Translation>{(t) => <p>{t("Select a configuration file to open")}</p>}</Translation>
                    </div>
                    <div className="message-body">
                        <div className="dropdown" id="dropdown-file">
                            <div className="dropdown-trigger">
                                <button
                                    className="button"
                                    aria-haspopup="true"
                                    aria-controls="dropdown-menu"
                                    onClick={showFilesDropDown}>
                                    <Translation>
                                        {(t) => <span id="dropdown-current-file-element">{t("new")}</span>}
                                    </Translation>
                                    <span className="icon is-small">
                                        <i className="fas fa-angle-down" aria-hidden="true"></i>
                                    </span>
                                </button>
                            </div>
                            <div className="dropdown-menu" id="dropdown-menu" role="menu">
                                <div className="dropdown-content">
                                    <Translation>
                                        {(t) => <a className="dropdown-item is-active">{t("new")}</a>}
                                    </Translation>
                                    <hr className="dropdown-divider" />
                                </div>
                            </div>
                        </div>
                        <div className="buttons-config-selector">
                            <button
                                className="button is-link"
                                id="button-just-notifications"
                                onClick={goToNotificationsPage}>
                                <span className="icon is-small">
                                    <i className="fas fa-bell"></i>
                                </span>
                                <Translation>{(t) => <span>{t("Just wanna see notifications")}</span>}</Translation>
                            </button>

                            <button
                                className="button is-link"
                                id="button-modal-make-copy-file"
                                onClick={copyFileAndSelectIt}>
                                <span className="icon is-small">
                                    <i className="fas fa-copy"></i>
                                </span>
                                <Translation>
                                    {(t) => <span>{t("Make a copy of this file and select it")}</span>}
                                </Translation>
                            </button>

                            <button className="button is-success" id="button-select-config-file" onClick={selectFile}>
                                <span className="icon is-small">
                                    <i className="fas fa-check"></i>
                                </span>
                                <span>Ok</span>
                            </button>
                        </div>
                    </div>
                </article>
            </div>
        </div>
    );
}

export default ModalSelectConfiguration;
