import React from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";

class ConfigurationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            configurations: props.configurations,
        };
    }

    addNewCamera() {
        throw "Method not implemented";
    }

    saveConfiguration() {
        throw "Method not implemented";
    }

    render() {
        return (
            <div className="" id="configuration-page">
                <div id="configurations">
                    <div className="tabs is-boxed">
                        <ul>
                            <li className="is-active" data-config="program" id="tab-program">
                                <a>
                                    <Translation>{(t) => <span>{t("Program configuration")}</span>}</Translation>
                                </a>
                            </li>
                        </ul>
                    </div>
                </div>
                <div className="buttons">
                    <button className="button is-link" id="button-add-new-camera" onClick={this.addNewCamera}>
                        <span className="icon is-small">
                            <i className="fas fa-plus"></i>
                        </span>
                        <Translation>{(t) => <span>{t("Add new camera")}</span>}</Translation>
                    </button>

                    <button className="button is-success" id="button-save-into-file" onClick={this.saveConfiguration}>
                        <span className="icon is-small">
                            <i className="fas fa-save"></i>
                        </span>
                        <Translation>{(t) => <span>{t("Save configurations into file")}</span>}</Translation>
                    </button>
                </div>
            </div>
        );
    }
}

ConfigurationPage.propTypes = {
    configurations: PropTypes.object.isRequired,
};
