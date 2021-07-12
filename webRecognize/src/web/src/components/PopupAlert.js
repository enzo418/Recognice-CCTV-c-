import React from "react";
import PropTypes from "prop-types";

const deleteAlert = (ev) => ev.target.parentNode.remove();

function PopupAlert(props) {
    const {alert} = props;

    return (
        <div className={"notification " + (alert.status === "ok" ? "is-success" : "is-danger")}>
            <button className="delete" onClick={deleteAlert}></button>
            <span className="notification-text">{alert.message}</span>
            <span className="notification-extra-text">{alert.extra}</span>
        </div>
    );
}

PopupAlert.propTypes = {
    alert: PropTypes.object.isRequired,
};

export default PopupAlert;
