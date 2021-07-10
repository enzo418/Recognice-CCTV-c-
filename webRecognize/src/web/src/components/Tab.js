import React from "react";
import PropTypes from "prop-types";

function Tab(props) {
    const {dataConfig} = props;
    return (
        <li data-config={dataConfig}>
            <a>{props.children}</a>
        </li>
    );
}

Tab.propTypes = {
    dataConfig: PropTypes.string.isRequired,
    children: PropTypes.object.isRequired,
};

export default Tab;
