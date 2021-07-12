import React from "react";
import PropTypes from "prop-types";
import {withTranslation} from "react-i18next";
import Tab from "./Tab";
import ProgramConfiguration from "./ProgramConfiguration";
import CameraConfiguration from "./CameraConfiguration";

class ConfigurationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            configurations: this.props.configurations.headers,
            currentTab: "program",
        };

        this.changeProgramTargetValue = this.changeProgramTargetValue.bind(this);
        this.changeCameraTargetValue = this.changeCameraTargetValue.bind(this);
    }

    addNewCamera() {
        throw "Method not implemented";
    }

    saveConfiguration() {
        throw "Method not implemented";
    }

    changeProgramTargetValue(target, value) {
        this.setState((prev) => {
            prev.configurations.program[target] = value;
            return prev;
        });
    }

    changeCameraTargetValue(id, target, value) {
        this.setState((prev) => {
            prev.configurations.cameras[id][target] = value;
            return prev;
        });
    }

    setActiveTab(tab) {
        this.setState((prev) => {
            prev.currentTab = tab;
            return prev;
        });
    }

    render() {
        const {t} = this.props;
        return (
            <div id="configuration-page">
                <div id="configurations">
                    <div className="tabs is-boxed">
                        <ul>
                            <Tab
                                isActive={this.state.currentTab === "program"}
                                onClick={() => this.setActiveTab("program")}>
                                <span>{t("Program configuration")}</span>
                            </Tab>
                            {this.state.configurations.cameras.map((camera) => (
                                <Tab
                                    isActive={camera.id === this.state.currentTab}
                                    key={camera.id}
                                    onClick={() => this.setActiveTab(camera.id)}>
                                    <span>{camera.cameraname}</span>
                                </Tab>
                            ))}
                        </ul>
                    </div>
                    {this.state.currentTab === "program" && (
                        <ProgramConfiguration
                            programConfiguration={this.state.configurations.program}
                            changeTargetValue={this.changeProgramTargetValue}
                            elements={this.props.elements.program}></ProgramConfiguration>
                    )}
                    {this.state.configurations.cameras.map(
                        (camera) =>
                            camera.id === this.state.currentTab && (
                                <CameraConfiguration
                                    key={camera.id}
                                    id={camera.id}
                                    cameraConfig={camera}
                                    changeTargetValue={this.changeCameraTargetValue}
                                    elements={this.props.elements.camera}></CameraConfiguration>
                            )
                    )}
                </div>
                <div className="buttons">
                    <button className="button is-link" id="button-add-new-camera" onClick={this.addNewCamera}>
                        <span className="icon is-small">
                            <i className="fas fa-plus"></i>
                        </span>
                        <span>{t("Add new camera")}</span>
                    </button>

                    <button className="button is-success" id="button-save-into-file" onClick={this.saveConfiguration}>
                        <span className="icon is-small">
                            <i className="fas fa-save"></i>
                        </span>
                        <span>{t("Save configurations into file")}</span>
                    </button>
                </div>
            </div>
        );
    }
}

ConfigurationPage.propTypes = {
    configurations: PropTypes.object.isRequired,
    elements: PropTypes.any.isRequired,
    t: PropTypes.func,
};

export default withTranslation()(ConfigurationPage);