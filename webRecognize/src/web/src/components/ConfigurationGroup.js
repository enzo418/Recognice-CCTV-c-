import React from "react";
import PropTypes from "prop-types";

import CheckBoxInput from "./Inputs/CheckBoxInput";
import NumberInput from "./Inputs/NumberInput";
import TextInput from "./Inputs/TextInput";

function ConfigurationGroup(props) {
    const {name, group} = props;
    return (
        <fieldset className="configuration-group">
            <legend>{name}</legend>
            {group.elements.map((element) => {
                let input;

                if (element.type === "number") {
                    input = <NumberInput></NumberInput>;
                } else if (element.type === "string") {
                    input = <TextInput></TextInput>;
                } else if (element.type === "boolean") {
                    input = <CheckBoxInput></CheckBoxInput>;
                }

                return input;
            })}
            {group.groups.map((group) => (
                <ConfigurationGroup key={group.name} name={group.name} group={group} />
            ))}
        </fieldset>
    );
}

ConfigurationGroup.propTypes = {
    name: PropTypes.string.isRequired,
    children: PropTypes.object.isRequired,
    group: PropTypes.object.isRequired,
};

export default ConfigurationGroup;
