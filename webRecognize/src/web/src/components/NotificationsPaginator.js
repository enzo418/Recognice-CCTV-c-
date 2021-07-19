import React from "react";
import PropTypes from "prop-types";
import NotificationGroup from "./NotificationGroup";
import Notification from "./Notification";
class NotificationsPaginator extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            index: props.lastNotificationIndex,
            lastIndex: 0,
            // groups: props.groups,
        };

        this.gotoIndex = this.gotoIndex.bind(this);
        this.previousNotification = this.previousNotification.bind(this);
        this.nextNotification = this.nextNotification.bind(this);
        this.gotoUttermost = this.gotoUttermost.bind(this);
        this.scrollContainerToElementTop = this.scrollContainerToElementTop.bind(this);
    }

    componentDidUpdate(prevProps, prevstate) {
        // console.log("pag." , {prevProps, prevstate, i: this.state.marker_notifications}, prevProps.notifications.length !== this.state.marker_notifications);
        // if(prevProps.notifications.length !== this.state.marker_notifications) {
        //     this.setState(() => ({index: this.state.lastIndex, marker_notifications: this.props.notifications.length}));
        // }
    }

    gotoIndex(i, cb = () => {}) {
        console.log("update:" , i);
        this.setState(() => ({index: i}), cb);
        this.props.onCurrentNotificationIndexChanged(i);
    }

    scrollContainerToElementTop() {
        // throw "Method not implemented";
    }

    // updates the current index to the next one and changes the current notification
    nextNotification() {
        var i = this.state.index < this.props.groups.length - 1 ? this.state.index + 1 : 0;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    // updates the current index to the previous and changes the current displayed
    previousNotification() {
        var i = this.state.index > 0 ? this.state.index - 1 : this.props.groups.length - 1;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    // go to the start or end of the notification collection
    gotoUttermost(end = true) {
        var i = end ? this.props.groups.length - 1 : 0;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    render() {
        return (
            <div className="notification-paginator">
                <div id="notifications-content" className="unselectable">
                    {this.props.groups.map(
                        (gr, i) =>
                            this.state.index === i && (
                                <NotificationGroup key={gr} group_id={gr}>
                                    {this.props.notifications
                                        .filter((not) => not.group_id === gr)
                                        .map((not, index) => (
                                            <Notification key={index} notification={not}></Notification>
                                        ))}
                                </NotificationGroup>
                            )
                    )}
                </div>

                {this.props.notifications.length > 0 && (
                    <div className="buttons-navigator-notification">
                        <button className="button navigator-notification buton-uttermost-start icon">
                            <i className="fas fa-angle-double-left"></i>
                        </button>

                        <button
                            className="navigator-notification previous-notification icon button"
                            onClick={this.previousNotification}>
                            <i className="fas fa-arrow-left is-left"></i>
                            <span className="notification-previous-left unselectable">{this.state.index}</span>
                        </button>

                        <button
                            className="button navigator-notification next-notification icon"
                            onClick={this.nextNotification}>
                            <span className="notification-next-left unselectable">
                                {this.props.groups.length - this.state.index}
                            </span>
                            <i className="fas fa-arrow-right is-right"></i>
                        </button>

                        <button className="button navigator-notification buton-uttermost-end icon">
                            <i className="fas fa-angle-double-right"></i>
                        </button>
                    </div>
                )}
            </div>
        );
    }
}

NotificationsPaginator.propTypes = {
    notifications: PropTypes.array.isRequired,
    groups: PropTypes.array.isRequired,
    lastNotificationIndex: PropTypes.number.isRequired,
    onCurrentNotificationIndexChanged: PropTypes.func.isRequired
};

export default NotificationsPaginator;
