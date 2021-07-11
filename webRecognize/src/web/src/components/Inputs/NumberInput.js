import React from "react";
import PropTypes from "prop-types";

class NumberInput extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            value: props.value,
        };
    }

    render() {
        const {name, placeHolder, value, label, hidden, tooltip} = this.props;
        var min,
            max = null;

        if ("min" in this.props) min = this.props.min;

        if ("max" in this.props) max = this.props.max;

        return (
            <div className={`card-content-item ${hidden && "is-hidden"}`} id={name.toLowerCase()} data-tip={tooltip}>
                <label htmlFor={name}>{label}</label>
                <input
                    className="input"
                    name={name}
                    type="number"
                    placeHolder={placeHolder}
                    min={min}
                    max={max}
                    value={value}></input>
            </div>
        );
    }
}

NumberInput.propTypes = {
    name: PropTypes.string.isRequired,
    placeHolder: PropTypes.string.isRequired,
    value: PropTypes.string.isRequired,
    label: PropTypes.string.isRequired,
    tooltip: PropTypes.string,
    hidden: PropTypes.bool,
    min: PropTypes.number,
    max: PropTypes.number,
};

export default NumberInput;
