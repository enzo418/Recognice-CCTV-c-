import React from "react";
import PropTypes from "prop-types";

function ConfigurationGroup(props) {
    const {id, name} = props;
    return (
        <fieldset className="configuration-group" id={id}>
            <legend>{name}</legend>
        </fieldset>
    );
}

ConfigurationGroup.propTypes = {
    id: PropTypes.number.isRequired,
    name: PropTypes.string.isRequired,
};
