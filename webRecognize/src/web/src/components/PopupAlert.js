import React from "react";
import PropTypes from "prop-types";

const deleteAlert = (ev) => ev.target.parentNode.remove();

function PopupAlert(props) {
    const {message, stateClass} = props;

    return (
        <div className={`notification ${stateClass}`}>
            <button className="delete" onClick={deleteAlert}></button>
            <span className="notification-text">{message}</span>
        </div>
    );
}

PopupAlert.propTypes = {
    stateClass: PropTypes.string.isRequired,
    message: PropTypes.string.isRequired,
};
