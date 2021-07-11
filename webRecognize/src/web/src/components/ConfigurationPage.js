import React from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";
import Tab from "./Tab";
import ProgramConfiguration from "./ProgramConfiguration";
import CameraConfiguration from "./CameraConfiguration";

class ConfigurationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            configurations: {
                cameras: [
                    {
                        id: 0,
                        cameraname: "camera1",
                        url: "rtsp://192.168.1.13:554?user=viewer&password=123&camera=2.sdp",
                    },
                    {
                        id: 1,
                        cameraname: "camera2",
                        url: "rtsp://192.168.1.13:554?user=viewer&password=123&camera=3.sdp",
                    },
                    {
                        id: 2,
                        cameraname: "camera3",
                        url: "rtsp://192.168.1.13:554?user=viewer&password=123&camera=4.sdp",
                    },
                ],
                program: {},
            },
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

    render() {
        return (
            <div id="configuration-page">
                <div id="configurations">
                    <div className="tabs is-boxed">
                        <ul>
                            <Tab dataConfig="program">
                                <Translation>{(t) => <span>{t("Program configuration")}</span>}</Translation>
                            </Tab>
                            {this.state.configurations.cameras.map((camera) => {
                                <Tab dataConfig="camera" data-index={camera.id}>
                                    <span>{camera.cameraname}</span>
                                </Tab>;
                            })}
                        </ul>
                    </div>
                    <ProgramConfiguration
                        programConfiguration={this.state.configurations.program}
                        changeTargetValue={this.changeProgramTargetValue}></ProgramConfiguration>
                    {this.state.configurations.cameras.map((camera) => (
                        <CameraConfiguration key={camera.id} id={camera.id} cameraConfig={camera}></CameraConfiguration>
                    ))}
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

export default ConfigurationPage;
