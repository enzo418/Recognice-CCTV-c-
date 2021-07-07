import React from 'react';
import PropTypes from 'prop-types';

class NumberInput extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            value: props.value,
        };
    }

    render() {
        const { name, placeholder, value, label, hidden, tooltip } = this.props;
        var min,
            max = null;

        // eslint-disable-next-line react/prop-types
        if ('min' in this.props) min = this.props.min;

        // eslint-disable-next-line react/prop-types
        if ('max' in this.props) max = this.props.max;

        return (
            <div className={`card-content-item ${hidden && 'is-hidden'}`} id={name.toLowerCase()} data-tip={tooltip}>
                <label htmlFor={name}>{label}</label>
                <input
                    className="input"
                    name={name}
                    type="number"
                    placeholder={placeholder}
                    min={min}
                    max={max}
                    value={value}
                ></input>
            </div>
        );
    }
}

NumberInput.propTypes = {
    name: PropTypes.string.isRequired,
    placeholder: PropTypes.string.isRequired,
    value: PropTypes.string.isRequired,
    label: PropTypes.string.isRequired,
    hidden: PropTypes.bool.isRequired,
    tooltip: PropTypes.string.isRequired,
};
