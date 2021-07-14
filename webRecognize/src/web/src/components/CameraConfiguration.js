import React from "react";
import PropTypes from "prop-types";

// the render prop
import {Translation} from "react-i18next";

import ConfigurationGroup from "./ConfigurationGroup";

class CameraConfiguration extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
        this.onAcceptModal = this.onAcceptModal.bind(this);
        this.openModalROI = this.openModalROI.bind(this);
        this.openModalAreas = this.openModalAreas.bind(this);
        this.openModalExclusivityAreas = this.openModalExclusivityAreas.bind(this);
    }

    onAcceptModal(id, value) {
        this.props.changeTargetValue(this.props.id, id, value);
    }

    openModalROI() {
        fetch(
            `/api/camera_frame?url=${encodeURIComponent(this.props.cameraConfig["url"])}&rotation=${
                this.props.cameraConfig["rotation"]
            }`
        )
            .then((res) => res.json())
            .then(({camera_frame}) => {
                this.props.openModalCanvas(
                    "roi",
                    (value) => this.onAcceptModal("roi", value),
                    camera_frame.frame,
                    this.props.cameraConfig["roi"]
                );
            });
    }

    openModalAreas() {
        fetch(
            `/api/camera_frame?url=${encodeURIComponent(this.props.cameraConfig["url"])}&rotation=${
                this.props.cameraConfig["rotation"]
            }&roi=${encodeURIComponent(this.props.cameraConfig["roi"])}`
        )
            .then((res) => res.json())
            .then(({camera_frame}) => {
                this.props.openModalCanvas(
                    "ignoredAreas",
                    (value) => this.onAcceptModal("ignoredareas", value),
                    camera_frame.frame,
                    this.props.cameraConfig["ignoredareas"]
                );
            });
    }

    openModalExclusivityAreas() {
        fetch(
            `/api/camera_frame?url=${encodeURIComponent(this.props.cameraConfig["url"])}&rotation=${
                this.props.cameraConfig["rotation"]
            }&roi=${encodeURIComponent(this.props.cameraConfig["roi"])}`
        )
            .then((res) => res.json())
            .then(({camera_frame}) => {
                this.props.openModalCanvas(
                    "exclusivityAreas",
                    (value) => this.onAcceptModal("pointsdiscriminators", value),
                    camera_frame.frame,
                    this.props.cameraConfig["pointsdiscriminators"]
                );
            });
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
                                <button className="button button-select-camera-roi" onClick={this.openModalROI}>
                                    {t("Select camera region of interest")}
                                </button>
                            )}
                        </Translation>

                        <Translation>
                            {(t) => (
                                <button
                                    className="button button-select-camera-ignored-areas"
                                    onClick={this.openModalAreas}>
                                    {t("Select camera ignored areas")}
                                </button>
                            )}
                        </Translation>

                        <Translation>
                            {(t) => (
                                <button
                                    className="button button-select-camera-exclusivity-areas"
                                    onClick={this.openModalExclusivityAreas}>
                                    {t("Select camera exclusivity areas")}
                                </button>
                            )}
                        </Translation>

                        <Translation>
                            {(t) => (
                                <button
                                    className="button is-danger button-delete-camera"
                                    onClick={() => this.props.deleteCamera(this.props.id)}>
                                    {t("Delete camera")}
                                </button>
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
    deleteCamera: PropTypes.func.isRequired,
    openModalCanvas: PropTypes.func.isRequired,
};

export default CameraConfiguration;
