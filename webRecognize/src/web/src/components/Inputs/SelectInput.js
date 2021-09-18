import React from "react";
import PropTypes from "prop-types";
import {withTranslation} from "react-i18next";

class SelectInput extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            value: props.value,
        };
    }

    render() {
        const {name, value, label, hidden, tooltip, options, t} = this.props;

        return (
            <div className={`card-content-item ${hidden && "is-hidden"}`} id={name.toLowerCase()} data-tip={tooltip}>
                <label htmlFor={name}>{label}</label>
                <div className="select">
                    <select name={name} value={value} onChange={({ target }) => this.props.onChange(target.value)}>
                        {Object.entries(options).map((entry) => (
                            <option key={entry[0]} value={entry[0]}>
                                {t(entry[1])}
                            </option>
                        ))}
                    </select>
                </div>
            </div>
        );
    }
}

SelectInput.propTypes = {
    name: PropTypes.string.isRequired,
    placeHolder: PropTypes.string,
    value: PropTypes.any,
    label: PropTypes.string.isRequired,
    tooltip: PropTypes.string,
    hidden: PropTypes.bool,
    min: PropTypes.number,
    max: PropTypes.number,
    onChange: PropTypes.func.isRequired,
    options: PropTypes.object.isRequired,
    t: PropTypes.func,
};

export default withTranslation()(SelectInput);
