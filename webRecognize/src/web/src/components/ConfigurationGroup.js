import React from "react";
import PropTypes from "prop-types";

import CheckBoxInput from "./Inputs/CheckBoxInput";
import NumberInput from "./Inputs/NumberInput";
import TextInput from "./Inputs/TextInput";

import {useTranslation} from "react-i18next";

function ConfigurationGroup(props) {
    const {t} = useTranslation();
    const {name, group, values, onChangeValue} = props;
    return (
        <fieldset className="configuration-group">
            <legend>{name}</legend>
            {group.elements.map((element) => {
                let input;

                if (element.type === "number") {
                    input = (
                        <NumberInput
                            name={element.target}
                            placeHolder={element.placeholder}
                            label={t(element.target)}
                            value={values[element.target]}
                            onChange={({target}) => onChangeValue(element.target, target.value)}></NumberInput>
                    );
                } else if (element.type === "string") {
                    input = (
                        <TextInput
                            name={element.target}
                            placeHolder={element.placeholder}
                            label={t(element.target)}
                            value={values[element.target]}
                            onChange={({target}) => onChangeValue(element.target, target.value)}></TextInput>
                    );
                } else if (element.type === "boolean") {
                    input = (
                        <CheckBoxInput
                            name={element.target}
                            placeHolder={element.placeholder}
                            label={t(element.target)}
                            value={values[element.target]}
                            onChange={({target}) => onChangeValue(element.target, target.checked)}></CheckBoxInput>
                    );
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
    group: PropTypes.object.isRequired,
    values: PropTypes.object.isRequired,
    onChangeValue: PropTypes.func.isRequired,
};

export default ConfigurationGroup;
