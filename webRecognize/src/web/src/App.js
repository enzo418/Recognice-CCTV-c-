import React from "react";
import ConfigurationPage from "./components/ConfigurationPage";
import NotificationPage from "./components/NotificationPage";
import HomeNavBar from "./components/HomeNavBar";
import {Switch, Route} from "react-router-dom";
import ModalSelectConfiguration from "./components/ModalSelectConfiguration";
import PropTypes from "prop-types";

import configuration_parser from "./modules/configuration_parser";
import Elements from "./elements.json";
import utils from "./utils/utils";
const elements = utils.elementsGroupsToLowerCase(Elements);

import i18n from "./i18n";
import PopupAlert from "./components/PopupAlert";
import ModalSelectFileName from "./components/ModalSelectFileName";

const pages = {
    configurations: {
        path: "/configurations",
        name: "configurations",
    },
    notifications: {
        path: "/notifications",
        name: "notifications",
    },
};

class App extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            recognize: {
                running: false,
                configuration: {
                    file: "",
                    headers: [],
                },
                toggle: () => this.toggleRecognize(),
                start: () => this.toggleRecognize("start"),
                stop: () => this.toggleRecognize("stop"),
            },
            // App config
            configuration: {
                set: (key, value) => this.changeConfiguration({key, value}),
                toggle: (key) => this.changeConfiguration({key: key}, true),
                sendPushOnNotification: false,
                playSoundOnNotification: true,
            },
            configurationFilesAvailables: [],
            error: null,
            alerts: [],
            fileNameToCopy: "", // this is used to know when the user wants to copy a cfg file
        };

        this.toggleRecognize = this.toggleRecognize.bind(this);
        this.changeConfiguration = this.changeConfiguration.bind(this);
        this.changeConfigurationFile = this.changeConfigurationFile.bind(this);
        this.addAlert = this.addAlert.bind(this);
        this.onWantsToCopyConfigurationFile = this.onWantsToCopyConfigurationFile.bind(this);
        this.callbackEnterFileName = this.callbackEnterFileName.bind(this);
        this.setLifeAlert = this.setLifeAlert.bind(this);
    }

    updateAlertsLife() {}

    componentDidMount() {
        fetch("/api/configuration_files")
            .then((res) => res.json())
            .then(({configuration_files}) => {
                this.setState(
                    () => {
                        return {configurationFilesAvailables: configuration_files};
                    },
                    (error) => {
                        this.setState(() => ({error}));
                    }
                );
            });

        let lang = window.localStorage.getItem("lang") || "en";
        i18n.changeLanguage(lang);
    }

    /**
     * Request the server to set the state of the recognizer
     * @param {string} to start|stop|toggle
     */
    toggleRecognize(to = "toggle") {
        if (["start", "stop", "toggle"].indexOf(to) === -1) {
            throw "Invalid recognizer state requested '" + to + "'";
        }
        to = to === "toggle" ? (this.state.recognize.running ? "stop" : "start") : to;
        let url = "/api/" + to + "_recognizer" + "?file_name=" + this.state.recognize.configuration.file;
        fetch(url)
            .then((res) => res.json())
            .then((res) => this.addAlert(res.status));
    }

    /**
     * Loads all the configuration from the file
     * @param {string} filename
     */
    changeConfigurationFile(filename) {
        this.setState((prev) => {
            prev.recognize.configuration.file = filename;
            return prev;
        });

        fetch(`/api/configuration_file?file=${filename}`)
            .then((res) => res.json())
            .then(({configuration_file}) => {
                this.setState((prev) => {
                    let configs = configuration_parser.parseConfiguration(configuration_file, elements);
                    configs.cameras.forEach((cam, i) => {
                        cam.id = i;
                    });

                    prev.recognize.configuration.headers = configs;

                    this.props.history.push(pages.configurations.path);

                    return prev;
                });
            });
    }

    saveConfigurationOnLocalStorage() {
        let notFunc = Object.keys(this.state.configuration).filter(
            (key) => typeof this.state.configuration[key] !== "function"
        );

        let values = Object.fromEntries(
            Object.entries(this.state.configuration).filter(([key]) => notFunc.includes(key))
        );

        window.localStorage.setItem("configuration", JSON.stringify(values));
    }

    /**
     * Changes the configuration of the app
     * @param {{string, number}} param0 obj value - key
     * @param {boolean} toggle optional, if should toggle the previous
     *                         value (only for boolean a key)
     */
    changeConfiguration({key, value}, toggle = null) {
        this.setState((prev) => {
            if (toggle) {
                if (typeof prev.configuration[key] !== "boolean") {
                    throw "ERROR: Cannot toggle a variable of type '" + typeof prev.configuration[key] + "'";
                }

                prev.configuration[key] = !prev.configuration[key];

                return prev;
            } else {
                return {key: value};
            }
        }, this.saveConfigurationOnLocalStorage);
    }

    callbackRemoveAlert(id) {
        this.setState((prev2) => {
            prev2.alerts.splice(id, 1);
            return prev2;
        });
    }

    addAlert(alert) {
        this.setState((prev) => {
            let id = prev.alerts.length;

            // remove alert after 3.5s
            let c = setTimeout(() => this.callbackRemoveAlert(id), 3500);

            // add alert to alerts
            prev.alerts.push({id, alert, timeout: c});

            return prev;
        });
    }

    setLifeAlert(id, extend) {
        if (extend) {
            this.setState((prev) => {
                let a = prev.alerts.find((a) => a.id === id);
                if (a) {
                    clearTimeout(a.timeout);
                }

                return prev;
            });
        } else {
            this.setState((prev) => {
                let a = prev.alerts.find((a) => a.id === id);
                if (a) {
                    a.tomeout = setTimeout(() => this.callbackRemoveAlert(id), 3500);
                }

                return prev;
            });
        }
    }

    onWantsToCopyConfigurationFile(filename) {
        this.setState(() => ({
            fileNameToCopy: filename,
        }));
    }

    callbackEnterFileName({cancelled, value}) {
        if (!cancelled) {
            fetch(`/api/copy_file?file=${this.state.fileNameToCopy}&copy_path=${value}`, {
                method: "POST",
                headers: {
                    "Content-Type": "multipart/form-data",
                },
                body: "",
            })
                .then((res) => res.json())
                .then((res) => this.addAlert(res.status));
        } else {
            this.setState(() => ({
                fileNameToCopy: "",
            }));
        }
    }

    render() {
        return (
            <div>
                <HomeNavBar pages={pages} recognize={this.state.recognize} toggleRecognize={this.toggleRecognize} />

                {this.state.recognize.configuration.file === "" &&
                    this.props.location.pathname !== pages.notifications.path &&
                    this.state.fileNameToCopy === "" && (
                        <ModalSelectConfiguration
                            configurationFilesAvailables={this.state.configurationFilesAvailables}
                            changeConfigurationFile={this.changeConfigurationFile}
                            onWantsToCopyConfigurationFile={this.onWantsToCopyConfigurationFile}
                        />
                    )}

                {this.state.fileNameToCopy !== "" && this.props.location.pathname !== pages.notifications.path && (
                    <ModalSelectFileName filename={this.state.fileNameToCopy} callback={this.callbackEnterFileName} />
                )}

                <Switch>
                    {this.state.recognize.configuration.file !== "" && (
                        <Route path={pages.configurations.path}>
                            <ConfigurationPage
                                elements={elements}
                                configurations={this.state.recognize.configuration}
                                addAlert={this.addAlert}></ConfigurationPage>
                        </Route>
                    )}

                    <Route path={pages.notifications.path}>
                        <NotificationPage configuration={this.state.configuration}></NotificationPage>
                    </Route>
                </Switch>

                <div id="alerts">
                    {this.state.alerts.map((el) => (
                        <PopupAlert key={el.id} id={el.id} setLifeAlert={this.setLifeAlert} alert={el.alert} />
                    ))}
                </div>
            </div>
        );
    }
}

App.propTypes = {
    location: PropTypes.object,
    history: PropTypes.any.isRequired,
};

export default App;
