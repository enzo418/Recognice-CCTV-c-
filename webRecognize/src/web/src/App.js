import React from "react";
import ConfigurationPage from "./components/ConfigurationPage";
import NotificationPage from "./components/NotificationPage";
import HomeNavBar from "./components/HomeNavBar";
import {BrowserRouter as Switch, Route} from "react-router-dom";

const pages = {
    configuration: {
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
            },
            configurations: {},
            configuration_files_availables: [],
        };

        this.toggleRecognize = this.toggleRecognize.bind(this);
    }

    componentDidMount() {
        // 1. fecth all the configurations availables into configuration_files_availables
    }

    toggleRecognize() {
        // this.setState((prevState) => ({
        //     recognizeIsRunning: !prevState.recognizeIsRunning,
        // }));
        // TODO: call the api
    }

    render() {
        return (
            <Switch>
                <HomeNavBar recognize={this.state.recognize} toggleRecognize={this.toggleRecognize} />

                <Route path={pages.configuration.path}>
                    <ConfigurationPage></ConfigurationPage>
                </Route>

                <Route path={pages.notifications.path}>
                    <NotificationPage></NotificationPage>
                </Route>
            </Switch>
        );
    }
}

App.propTypes = {};

export default App;
