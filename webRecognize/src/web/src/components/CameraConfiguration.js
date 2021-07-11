import React from "react";
import PropTypes from "prop-types";

// the render prop
import {Translation} from "react-i18next";

import ConfigurationGroup from "./ConfigurationGroup";

class CameraConfiguration extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
    }

    render() {
        const {id, cameraConfig} = this.props;

        return (
            <div className="card">
                <header className="card-header">
                    <p className="card-header-title">{cameraConfig["cameraname"]}</p>
                </header>

                <div className="card-content camera-config-content">
                    {this.props.elements.groups.map((group) => (
                        <ConfigurationGroup
                            key={group.name}
                            name={group.name}
                            group={group}
                            values={cameraConfig}
                            onChangeValue={(target, value) => this.props.changeTargetValue(id, target, value)}
                        />
                    ))}
                </div>

                <footer className="card-footer">
                    <div className="card-footer-item footer-camera-buttons">
                        <Translation>
                            {(t) => (
                                <button className="button button-select-camera-roi">
                                    {t("Select camera region of interest")}
                                </button>
                            )}
                        </Translation>

                        <Translation>
                            {(t) => (
                                <button className="button button-select-camera-ignored-areas">
                                    {t("Select camera ignored areas")}
                                </button>
                            )}
                        </Translation>

                        <Translation>
                            {(t) => (
                                <button className="button button-select-camera-exclusivity-areas">
                                    {t("Select camera exclusivity areas")}
                                </button>
                            )}
                        </Translation>

                        <Translation>
                            {(t) => (
                                <button className="button is-danger button-delete-camera">{t("Delete camera")}</button>
                            )}
                        </Translation>
                    </div>
                </footer>
            </div>
        );
    }
}

CameraConfiguration.propTypes = {
    id: PropTypes.number.isRequired,
    cameraConfig: PropTypes.object.isRequired,
    changeTargetValue: PropTypes.func.isRequired,
    elements: PropTypes.object.isRequired,
};

export default CameraConfiguration;
