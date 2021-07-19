import React from "react";
import PropTypes from "prop-types";
import moment from "moment";
import {Translation} from "react-i18next";
import NotificationsPaginator from "./NotificationsPaginator";
import bulmaCalendar from "bulma-calendar/dist/js/bulma-calendar.min";
import "../../node_modules/bulma-calendar/dist/css/bulma-calendar.min.css";

const parseStringToDate = (datestring, return_moment = false) => {
    var parsed = moment(datestring, "DD_MM_YYYY_hh_mm_ss");
    return return_moment ? parsed : parsed.toDate();
};

class NotificationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            allNotifications: [], // all available notifications
            showingNotifications: [], // notifications being shown
            calendar: null,
        };

        this.calendar_OnSelect = this.calendar_OnSelect.bind(this);
        this.calendar_OnCancel = this.calendar_OnCancel.bind(this);
        this.updateCalendarLimitis = this.updateCalendarLimitis.bind(this);
        this.getGroups = this.getGroups.bind(this);
    }

    reloadCalendar(minDate, maxDate) {
        // calendar excludes the minDate...
        minDate = moment(minDate).subtract(1, "day").toDate();

        // Initialize all input of date type.
        const calendars = bulmaCalendar.attach('[type="date"]', {
            color: "primary",
            isRange: true,
            allowSameDayRange: true,
            lang: "en-US",
            startDate: undefined,
            endDate: undefined,
            minDate: minDate,
            maxDate: maxDate,
            disabledDates: [],
            disabledWeekDays: undefined,
            highlightedDates: [],
            weekStart: 0,
            dateFormat: "dd/MM/yyyy",
            navigationMonthFormat: "MMMM",
            navigationYearFormat: "yyy",
            enableMonthSwitch: true,
            enableYearSwitch: true,
            displayYearsCount: 50,
        });

        // Loop on each calendar initialized
        calendars.forEach((calendar) => {
            // Add listener to select event
            calendar.on("select", this.calendar_OnSelect);
            calendar.on("save", this.calendar_OnCancel);
        });

        this.setState(() => ({calendar: calendars[0]}));
    }

    updateCalendarLimitis() {
        var minDate, maxDate;

        if (this.state.allNotifications.length > 0) {
            minDate = this.state.allNotifications[0].datetime;
            maxDate = this.state.allNotifications[this.state.allNotifications.length - 1].datetime;
        }

        if (!this.state.calendar || !this.state.calendar.isOpen()) {
            // Initialize all input of date type.
            this.reloadCalendar(minDate, maxDate);
        }
    }

    calendar_OnSelect(e) {
        var start = e.data.date.start,
            end = e.data.date.end;

        this.setState((prev) => ({
            showingNotifications: prev.allNotifications.filter((not) => not.datetime >= start && not.datetime <= end),
        }));
    }

    calendar_OnCancel(e) {
        if (!e.data.datePicker.date.start && !e.data.datePicker.date.end && this.state.allNotifications) {
            // notificationPaginator.index = 0;
            this.setState((prev) => ({showingNotifications: prev.allNotifications}));

            // if (this.state.allNotifications === 0) {
            // } else {
            // notificationPaginator.gotoIndex(0);
            // }
        }
    }

    componentDidMount() {
        fetch("/api/notifications")
            .then((res) => res.json())
            .then((res) => {
                res.notifications.forEach((not) => {
                    not.datetime = parseStringToDate(not.datetime);
                });

                this.setState(
                    () => ({allNotifications: res.notifications, showingNotifications: res.notifications}),
                    this.updateCalendarLimitis
                );
            });
    }

    getGroups() {
        return [...this.state.showingNotifications.reduce((ac, not) => ac.add(not.group_id), new Set())];
    }

    render() {
        return (
            <div id="notifications-page">
                <div className="header">
                    <Translation>{(t) => <h1 className="title">{t("Notifications")}</h1>}</Translation>

                    <div className="datetimepicker-container">
                        <input type="date" />
                    </div>

                    <div className="notifications-configuration">
                        <label
                            className={`notifications-configuration-item checkbox ${
                                this.props.configuration.sendPushOnNotification ? "enabled" : "disabled"
                            }`}
                            onClick={() => this.props.configuration.toggle("sendPushOnNotification")}>
                            <i className="fas fa-bell"></i>
                            <Translation>{(t) => <p>{t("Send push notifications")}</p>}</Translation>
                        </label>

                        <label
                            className={`notifications-configuration-item checkbox ${
                                this.props.configuration.playSoundOnNotification ? "enabled" : "disabled"
                            }`}
                            onClick={() => this.props.configuration.toggle("playSoundOnNotification")}>
                            <i className="fas fa-volume-off"></i>
                            <Translation>{(t) => <p>{t("Play sound on notification")}</p>}</Translation>
                        </label>
                    </div>
                </div>

                <NotificationsPaginator
                    notifications={this.state.showingNotifications}
                    groups={this.getGroups()}></NotificationsPaginator>
            </div>
        );
    }
}

NotificationPage.propTypes = {
    configuration: PropTypes.object.isRequired,
};

export default NotificationPage;
