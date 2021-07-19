import React from "react";
import PropTypes from "prop-types";

function NotificationGroup(props) {
    return (
        <div id={props.group_id} className="box">
            {props.children}
        </div>
    );
}

NotificationGroup.propTypes = {
    group_id: PropTypes.string.isRequired,
    children: PropTypes.any,
};

export default NotificationGroup;
