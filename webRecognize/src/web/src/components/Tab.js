import React from "react";
import PropTypes from "prop-types";

function Tab(props) {
    const {isActive, onClick} = props;
    return (
        <li className={isActive ? "tab is-active" : "tab"}>
            <a onClick={onClick}>{props.children}</a>
        </li>
    );
}

Tab.propTypes = {
    children: PropTypes.object.isRequired,
    onClick: PropTypes.func.isRequired,
    isActive: PropTypes.bool.isRequired,
};

export default Tab;
