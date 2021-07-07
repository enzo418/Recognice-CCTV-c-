import React from "react";
import PropTypes from "prop-types";

// the render prop
import {Translation} from "react-i18next";

class CameraConfiguration extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
    }

    render() {
        const {i, cameraConfig} = this.props;

        return (
            <div className="card is-hidden" id={`camera-${i}`}>
                <header className="card-header">
                    <p className="card-header-title">{cameraConfig["cameraname"]}</p>
                </header>

                <div className="card-content camera-config-content"></div>

                <div className="card-footer">
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
                </div>
            </div>
        );
    }
}

CameraConfiguration.propTypes = {
    i: PropTypes.number.isRequired,
    cameraConfig: PropTypes.object.isRequired,
};
