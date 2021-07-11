import React from "react";
import ConfigurationPage from "./components/ConfigurationPage";
import NotificationPage from "./components/NotificationPage";
import HomeNavBar from "./components/HomeNavBar";
import {Switch, Route} from "react-router-dom";
import ModalSelectConfiguration from "./components/ModalSelectConfiguration";
import PropTypes from "prop-types";

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
        };

        this.toggleRecognize = this.toggleRecognize.bind(this);
        this.changeConfiguration = this.changeConfiguration.bind(this);
    }

    componentDidMount() {
        // 1. fecth all the configurations availables into configurationFilesAvailables
        // 2. map each one to {id: #, file: str}
    }

    toggleRecognize(to = "toggle") {
        console.log(to);
        // throw "Not implemented";
        // this.setState((prevState) => ({
        //     recognizeIsRunning: !prevState.recognizeIsRunning,
        // }));
        // TODO: call the api
    }

    /**
     * Loads all the configuration from the file
     * @param {number} fileIndex
     */
    changeConfigurationFile(fileIndex) {
        console.log(fileIndex);
        throw "Not implemented";
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

    render() {
        return (
            <div>
                <HomeNavBar pages={pages} recognize={this.state.recognize} toggleRecognize={this.toggleRecognize} />

                {this.state.configuration.file === "" && this.props.location.pathname !== pages.notifications.path && (
                    <ModalSelectConfiguration
                        configurationFilesAvailables={this.state.configurationFilesAvailables}
                        changeConfigurationFile={this.changeConfigurationFile}
                    />
                )}

                <Switch>
                    {/* {this.state.configuration.file !== "" && ( */}
                    <Route path={pages.configurations.path}>
                        <ConfigurationPage configurations={this.state.recognize.configuration}></ConfigurationPage>
                    </Route>
                    {/* )} */}

                    <Route path={pages.notifications.path}>
                        <NotificationPage configuration={this.state.configuration}></NotificationPage>
                    </Route>
                </Switch>
            </div>
        );
    }
}

App.propTypes = {
    location: PropTypes.object,
};

export default App;
