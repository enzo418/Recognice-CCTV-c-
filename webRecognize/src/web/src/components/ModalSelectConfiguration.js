import React from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";
import {Link} from "react-router-dom";

import Modal from "./Modal";

class ModalSelectConfiguration extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            file: "new",
        };
    }
    showFilesDropDown() {
        throw "Method not implemented";
    }

    goToNotificationsPage() {
        throw "Method not implemented";
    }

    copyFileAndSelectIt() {
        throw "Method not implemented";
    }

    selectFile() {
        throw "Method not implemented";
    }

    onChangeFile(file) {
        this.setState(() => ({file}));
    }

    render() {
        let header = <Translation>{(t) => <p>{t("Select a configuration file to open")}</p>}</Translation>;
        let body = (
            <select value={this.state.file} onChange={({target}) => this.onChangeFile(target.value)}>
                <Translation>
                    {(t) => (
                        <option value="new" className="dropdown-item is-active">
                            {t("new")}
                        </option>
                    )}
                </Translation>
                {this.props.configurationFilesAvailables.map((cfg) => {
                    <option key={cfg.id} value={cfg.id} className="dropdown-item">
                        {cfg.file}
                    </option>;
                })}
            </select>
        );
        let footer = (
            <div>
                <Translation>{(t) => <Link to="/notifications">{t("Just wanna see notifications")}</Link>}</Translation>

                <button className="button is-link" id="button-modal-make-copy-file" onClick={this.copyFileAndSelectIt}>
                    <span className="icon is-small">
                        <i className="fas fa-copy"></i>
                    </span>
                    <Translation>{(t) => <span>{t("Make a copy of this file and select it")}</span>}</Translation>
                </button>

                <button
                    className="button is-success"
                    id="button-select-config-file"
                    onClick={() => this.props.changeConfigurationFile(this.state.file)}>
                    <span className="icon is-small">
                        <i className="fas fa-check"></i>
                    </span>
                    <span>Ok</span>
                </button>
            </div>
        );
        return <Modal header={header} body={body} footer={footer} />;
    }
}

ModalSelectConfiguration.propTypes = {
    configurationFilesAvailables: PropTypes.array.isRequired,
    changeConfigurationFile: PropTypes.func.isRequired,
};

export default ModalSelectConfiguration;
