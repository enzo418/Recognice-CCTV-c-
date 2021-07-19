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
import CanvasRoiHandler from "./modules/canvas/canvas_handler_roi";
import CanvasAreasHandler from "./modules/canvas/canvas_handler_area";
import CanvasExclusivityAreasHandler from "./modules/canvas/canvas_handler_exclAreas";
import {w3cwebsocket as W3CWebSocket} from "websocket";

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

const client = new W3CWebSocket("ws://localhost:3001/recognize");

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
            configuration: Object.assign(
                {
                    set: (key, value) => this.changeConfiguration({key, value}),
                    toggle: (key) => this.changeConfiguration({key: key}, true),
                },
                JSON.parse(window.localStorage.getItem("configuration")) || {
                    sendPushOnNotification: false,
                    playSoundOnNotification: true,
                }
            ),
            configurationFilesAvailables: [],
            error: null,
            alerts: [],
            fileNameToCopy: "", // this is used to know when the user wants to copy a cfg file
            modalCanvas: {
                references: {
                    roi: React.createRef(),
                    ignoredAreas: React.createRef(),
                    exclusivityAreas: React.createRef(),
                },

                currentImage: "", // base64 encoded image to use

                initialValue: "",

                // saves the current handler
                // id is used to access references[activeHandlerId]
                activeHandlerId: "",

                // default handler, can change
                onAccept: () => this.hideCanvasModal(),

                // do not change cancel
                onCancel: () => this.hideCanvasModal(),
            },
            notifications: [],
        };

        this.toggleRecognize = this.toggleRecognize.bind(this);
        this.changeConfiguration = this.changeConfiguration.bind(this);
        this.changeConfigurationFile = this.changeConfigurationFile.bind(this);
        this.addAlert = this.addAlert.bind(this);
        this.onWantsToCopyConfigurationFile = this.onWantsToCopyConfigurationFile.bind(this);
        this.callbackEnterFileName = this.callbackEnterFileName.bind(this);
        this.setLifeAlert = this.setLifeAlert.bind(this);
        this.openModalCanvas = this.openModalCanvas.bind(this);
        this.onAcceptModalCanvas = this.onAcceptModalCanvas.bind(this);
        this.hideCanvasModal = this.hideCanvasModal.bind(this);
    }

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

        fetch("/api/notifications")
            .then((res) => res.json())
            .then((res) => {
                this.setState(() => ({notifications: utils.prepareNotifications(res.notifications)}));
            });

        client.onmessage = (message) => {
            console.log(message);
            if (message.notifications) {
                this.setState((prev) => ({
                    notifications: prev.notifications.concat(utils.prepareNotifications(message.notifications)),
                }));
            }
        };
    }

    hideCanvasModal() {
        this.setState((prev) => {
            prev.modalCanvas.activeHandlerId = "";
            return prev;
        });
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

    parseAndLoadNewConfigurationFile(filename, configurationFile) {
        this.setState((prev) => {
            prev.fileNameToCopy = "";
            prev.recognize.configuration.file = filename;

            let configs = configuration_parser.parseConfiguration(configurationFile, elements);
            configs.cameras.forEach((cam, i) => {
                cam.id = i;
            });

            prev.recognize.configuration.headers = configs;

            this.props.history.push(pages.configurations.path);

            return prev;
        });
    }

    /**
     * Loads all the configuration from the file
     * @param {string} filename
     */
    changeConfigurationFile(filename) {
        fetch(`/api/configuration_file?file=${filename}`)
            .then((res) => res.json())
            .then(({configuration_file}) => this.parseAndLoadNewConfigurationFile(filename, configuration_file));
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
            let filename = value.indexOf(".ini") >= 0 ? value : value + ".ini";
            fetch(`/api/copy_file?file=${this.state.fileNameToCopy}&copy_path=${filename}`, {
                method: "POST",
                headers: {
                    "Content-Type": "multipart/form-data",
                },
                body: "",
            })
                .then((res) => res.json())
                .then(({configuration_file}) => this.parseAndLoadNewConfigurationFile(filename, configuration_file));
        } else {
            this.setState(() => ({
                fileNameToCopy: "",
            }));
        }
    }

    /**
     * Opens a modal that contains a canvas
     * @param {string} canvasType roi|ignoredAreas|exclusivityAreas
     * @param {Function} onAccept callback when the user hits "Ok"
     * @param {string} image base64 encoded image
     */
    openModalCanvas(canvasType = "roi", onAccept, image, initialValue) {
        this.setState((prev) => {
            prev.modalCanvas.onAccept = onAccept;
            prev.modalCanvas.activeHandlerId = canvasType;
            prev.modalCanvas.currentImage = image;
            prev.modalCanvas.initialValue = initialValue;
            return prev;
        });
    }

    onAcceptModalCanvas() {
        // get value generated from the user input
        let value = this.state.modalCanvas.references[this.state.modalCanvas.activeHandlerId].current.getValue();

        // pass the accepted value to the canvas
        this.state.modalCanvas.onAccept(value);

        //
        this.hideCanvasModal();
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

                {this.state.modalCanvas.activeHandlerId === "roi" && (
                    <CanvasRoiHandler
                        ref={this.state.modalCanvas.references.roi}
                        image={this.state.modalCanvas.currentImage}
                        initialValue={this.state.modalCanvas.initialValue}
                        onAccept={this.onAcceptModalCanvas}
                        onCancel={this.hideCanvasModal}></CanvasRoiHandler>
                )}

                {this.state.modalCanvas.activeHandlerId === "ignoredAreas" && (
                    <CanvasAreasHandler
                        ref={this.state.modalCanvas.references.ignoredAreas}
                        image={this.state.modalCanvas.currentImage}
                        initialValue={this.state.modalCanvas.initialValue}
                        onAccept={this.onAcceptModalCanvas}
                        onCancel={this.hideCanvasModal}
                    />
                )}

                {this.state.modalCanvas.activeHandlerId === "exclusivityAreas" && (
                    <CanvasExclusivityAreasHandler
                        ref={this.state.modalCanvas.references.exclusivityAreas}
                        image={this.state.modalCanvas.currentImage}
                        initialValue={this.state.modalCanvas.initialValue}
                        onAccept={this.onAcceptModalCanvas}
                        onCancel={this.hideCanvasModal}
                    />
                )}

                <Switch>
                    {this.state.recognize.configuration.file !== "" && (
                        <Route path={pages.configurations.path}>
                            <ConfigurationPage
                                elements={elements}
                                configurations={this.state.recognize.configuration}
                                addAlert={this.addAlert}
                                openModalCanvas={this.openModalCanvas}></ConfigurationPage>
                        </Route>
                    )}

                    <Route path={pages.notifications.path}>
                        <NotificationPage
                            notifications={this.state.notifications}
                            configuration={this.state.configuration}></NotificationPage>
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
