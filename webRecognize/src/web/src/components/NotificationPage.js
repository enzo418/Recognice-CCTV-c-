import React from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";
import NotificationsPaginator from "./NotificationsPaginator";

class NotificationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
    }

    togglePushNotifications() {
        throw "Method not implemented";
    }

    toggleSoundNotification() {
        throw "Method not implemented";
    }

    render() {
        return (
            <div id="notifications-page" className="is-hidden">
                <div className="header">
                    <Translation>{(t) => <h1 className="title">{t("Notifications")}</h1>}</Translation>

                    <div className="datetimepicker-container">
                        <input type="date" />
                    </div>

                    <div className="notifications-configuration">
                        <label className="notifications-configuration-item checkbox">
                            <input type="checkbox" id="toggle-push" checked onClick={this.togglePushNotifications} />
                            <i className="fas fa-bell"></i>
                            <Translation>{(t) => <p>{t("Send push notifications")}</p>}</Translation>
                        </label>

                        <label className="notifications-configuration-item checkbox">
                            <input
                                type="checkbox"
                                id="toggle-notification-sound"
                                checked
                                onClick={this.toggleSoundNotification}
                            />
                            <i className="fas fa-volume-off"></i>
                            <Translation>{(t) => <p>{t("Play sound on notification")}</p>}</Translation>
                        </label>
                    </div>
                </div>

                {NotificationsPaginator}
            </div>
        );
    }
}

NotificationPage.propTypes = {};
