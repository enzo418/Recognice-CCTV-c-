import React from "react";
import PropTypes from "prop-types";
import moment from "moment";
import {Translation} from "react-i18next";
import NotificationsPaginator from "./NotificationsPaginator";
import bulmaCalendar from "bulma-calendar/dist/js/bulma-calendar.min";
import utils from "../utils/utils";

const getHoursMinute = (date) => `${date.getHours()}:${date.getMinutes()}:00`;
const getMomentTime = (date) => moment(getHoursMinute(date), 'hh:mm:ss');

class NotificationPage extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            notifications: [], // all the notifications
            showingNotifications: [], // notifications being shown
            lastNotificationsFilteredCalendar: [],
            calendar: null,
            timePicker: null,
        };

        this.calendar_OnSelect = this.calendar_OnSelect.bind(this);
        this.calendar_OnCancel = this.calendar_OnCancel.bind(this);
        this.timePicker_OnCancel = this.timePicker_OnCancel.bind(this);
        this.timePicker_OnSelect = this.timePicker_OnSelect.bind(this);
        this.updateCalendarLimitis = this.updateCalendarLimitis.bind(this);
        this.getGroups = this.getGroups.bind(this);
    }

    reloadCalendar(minDate, maxDate) {
        // avoid reloading while it's open
        if (this.state.calendar && this.state.calendar.isOpen()) return;

        if (!minDate && !maxDate) {
            minDate = maxDate = new Date();
        }

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

        if (this.state.notifications.length > 0) {
            minDate = this.state.notifications[0].datetime;
            maxDate = this.state.notifications[this.state.notifications.length - 1].datetime;
        }

        if (!this.state.calendar || !this.state.calendar.isOpen()) {
            // Initialize all input of date type.
            this.reloadCalendar(minDate, maxDate);
        }
    }

    calendar_OnSelect(e) {
        var start = e.data.date.start,
            end = e.data.date.end;

        this.setState((prev) => {
            prev.showingNotifications = this.state.notifications.filter(
                (not) => not.datetime >= start && not.datetime <= end
            );

            prev.lastNotificationsFilteredCalendar = prev.showingNotifications;

            // set time picker
            if (prev.showingNotifications.length > 0) {
                const timePickers = bulmaCalendar.attach('[type="time"]', {
                    color: "primary",
                    isRange: true,
                    lang: "en-US",
                    timeFormat: "HH:mm",
                });

                timePickers.forEach((timep) => {
                    // Add listener to select event
                    timep.on("select", this.timePicker_OnSelect);
                    timep.on("save", this.timePicker_OnCancel);
                });

                prev.timePicker = timePickers[0];
            }
        }, () => this.forceUpdate());
    }

    calendar_OnCancel(e) {
        if (!e.data.datePicker.date.start && !e.data.datePicker.date.end && this.state.notifications) {
            // notificationPaginator.index = 0;
            this.setState(() => ({showingNotifications: []}));

            // if (this.state.notifications === 0) {
            // } else {
            // notificationPaginator.gotoIndex(0);
            // }
        }
    }

    timePicker_OnSelect() {
    }

    timePicker_OnCancel(e) {
        var end = getMomentTime(e.data.timePicker.end);
        var start = getMomentTime(e.data.timePicker.start);

        console.log({start, end, e});

        this.setState((prev) => {
            prev.showingNotifications = prev.lastNotificationsFilteredCalendar.filter(
                (not) => getMomentTime(not.datetime).isBetween(start, end)
            );
        }, () => this.forceUpdate());
    }

    componentDidMount() {
        fetch("/api/notifications")
            .then((res) => res.json())
            .then((res) => {
                this.setState(() => ({notifications: utils.prepareNotifications(res.notifications)}));
            });

        this.props.socket.on("notifications", (notifications) => {
            if (notifications) {
                this.setState(
                    (prev) => ({
                        notifications: prev.notifications.concat(utils.prepareNotifications(notifications)),
                    })
                );
            }
        });

        this.updateCalendarLimitis();
    }

    componentDidUpdate() {
        // if calendar is not showing and there is no selected date:
        // this.updateCalendarLimitis();
    }

    getGroups(nots) {
        return [...nots.reduce((ac, not) => ac.add(not.group_id), new Set())];
    }

    render() {
        return (
            <div id="notifications-page">
                <div className="header">
                    <Translation>{(t) => <h1 className="title">{t("Notifications")}</h1>}</Translation>

                    <div className="datetimepicker-container" onClick={this.updateCalendarLimitis}>
                        <input type="date" />
                        <div className={(this.state.showingNotifications.length > 0 ? "" : "is-hidden")} >
                            <input type="time" />
                        </div>
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
                    notifications={this.state.showingNotifications.length > 0 ? this.state.showingNotifications : this.state.notifications}
                    groups={this.getGroups(this.state.showingNotifications.length > 0 ? this.state.showingNotifications : this.state.notifications)}></NotificationsPaginator>
            </div>
        );
    }
}

NotificationPage.propTypes = {
    configuration: PropTypes.object.isRequired,
    socket: PropTypes.object.isRequired
};

export default NotificationPage;
