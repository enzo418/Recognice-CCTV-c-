import React from "react";
import PropTypes from "prop-types";

class NotificationsPaginator extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            index: 0,
        };
    }

    gotoIndex() {
        throw "Method not implemented";
    }

    scrollContainerToElementTop() {
        throw "Method not implemented";
    }

    // updates the current index to the next one and changes the current notification
    nextNotification() {
        var i = this.index < this.props.notifications.length - 1 ? this.index + 1 : 0;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    // updates the current index to the previous and changes the current displayed
    previousNotification() {
        var i = this.index > 0 ? this.index - 1 : this.props.notifications.length - 1;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    // go to the start or end of the notification collection
    gotoUttermost(end = true) {
        var i = end ? this.props.notifications.length - 1 : 0;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    render() {
        return (
            <div className="notification-paginator">
                <div id="notifications-content" className="unselectable"></div>

                <div className="buttons-navigator-notification">
                    <button className="button navigator-notification buton-uttermost-start icon is-hidden">
                        <i className="fas fa-angle-double-left"></i>
                    </button>

                    <button
                        className="navigator-notification previous-notification icon button is-hidden"
                        onClick={this.previousNotification}>
                        <i className="fas fa-arrow-left is-left"></i>
                        <span className="notification-previous-left unselectable"></span>
                    </button>

                    <button
                        className="button navigator-notification next-notification icon is-hidden"
                        onClick={this.nextNotification}>
                        <span className="notification-next-left unselectable"></span>
                        <i className="fas fa-arrow-right is-right"></i>
                    </button>

                    <button className="button navigator-notification buton-uttermost-end icon is-hidden">
                        <i className="fas fa-angle-double-right"></i>
                    </button>
                </div>
            </div>
        );
    }
}

NotificationsPaginator.propTypes = {
    notifications: PropTypes.array.isRequired,
};

export default NotificationsPaginator;
