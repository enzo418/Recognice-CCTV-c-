import React from "react";
import PropTypes from "prop-types";

import CheckBoxInput from "./Inputs/CheckBoxInput";
import NumberInput from "./Inputs/NumberInput";
import TextInput from "./Inputs/TextInput";

import {withTranslation} from "react-i18next";
import SelectInput from "./Inputs/SelectInput";

class ConfigurationGroup extends React.Component {
    constructor(props) {
        super(props);
    }

    render() {
        const {name, group, values, onChangeValue, t} = this.props;
        return (
            <fieldset className="configuration-group">
                <legend>{name}</legend>
                {group.elements.map((element) => {
                    let input;

                    if (element.type === "number") {
                        input = (
                            <NumberInput
                                key={element.target}
                                name={element.target}
                                placeHolder={element.placeholder}
                                label={t(element.target).label}
                                value={values[element.target] || 0}
                                onChange={(value) => onChangeValue(element.target, value)}
                                hidden={element.hidden}
                                tooltip={t(element.target).description}></NumberInput>
                        );
                    } else if (element.type === "string") {
                        input = (
                            <TextInput
                                key={element.target}
                                name={element.target}
                                placeHolder={element.placeholder}
                                label={t(element.target).label}
                                value={values[element.target] || ""}
                                onChange={(value) => onChangeValue(element.target, value)}
                                hidden={element.hidden}
                                tooltip={t(element.target).description}></TextInput>
                        );
                    } else if (element.type === "boolean") {
                        input = (
                            <CheckBoxInput
                                key={element.target}
                                name={element.target}
                                placeHolder={element.placeholder}
                                label={t(element.target).label}
                                checked={values[element.target] || false}
                                onChange={(value) => onChangeValue(element.target, value)}
                                hidden={element.hidden}
                                tooltip={t(element.target).description}></CheckBoxInput>
                        );
                    } else if (element.type === "select") {
                        input = (
                            <SelectInput
                                key={element.target}
                                name={element.target}
                                placeHolder={element.placeholder}
                                label={t(element.target).label}
                                value={values[element.target] || ""}
                                onChange={(value) =>
                                    onChangeValue(element.target, element.is_integer_value ? parseInt(value) : value)
                                }
                                hidden={element.hidden}
                                options={element.options}
                                tooltip={t(element.target).description}></SelectInput>
                        );
                    }

                    return input;
                })}
                {group.groups &&
                    group.groups.map((group) => (
                        <ConfigurationGroup
                            key={group.name}
                            name={group.name}
                            group={group}
                            values={values}
                            onChangeValue={onChangeValue}
                            t={t}
                        />
                    ))}
            </fieldset>
        );
    }
}

ConfigurationGroup.propTypes = {
    name: PropTypes.string.isRequired,
    group: PropTypes.object.isRequired,
    values: PropTypes.object.isRequired,
    onChangeValue: PropTypes.func.isRequired,
    t: PropTypes.func,
};

export default withTranslation()(ConfigurationGroup);
