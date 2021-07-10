import React from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";
import Tab from "./Tab";
import ProgramConfiguration from "./ProgramConfiguration copy";

class ConfigurationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            configurations: [],
        };

        // test values
        this.setState(() => ({
            configurations: {
                cameras: [
                    {id: 0, name: "camera1", url: "rtsp://192.168.1.13:554?user=viewer&password=123&camera=2.sdp"},
                    {id: 1, name: "camera2", url: "rtsp://192.168.1.13:554?user=viewer&password=123&camera=3.sdp"},
                    {id: 2, name: "camera3", url: "rtsp://192.168.1.13:554?user=viewer&password=123&camera=4.sdp"},
                ],
            },
        }));
    }

    addNewCamera() {
        throw "Method not implemented";
    }

    saveConfiguration() {
        throw "Method not implemented";
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
                                    <span>camera.name</span>
                                </Tab>;
                            })}
                        </ul>
                    </div>
                    <ProgramConfiguration configuration={this.state.configurations.program}></ProgramConfiguration>
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
