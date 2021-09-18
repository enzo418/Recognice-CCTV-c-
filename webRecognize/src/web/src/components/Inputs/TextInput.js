import React from "react";
import PropTypes from "prop-types";

class TextInput extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            value: props.value,
        };
    }

    render() {
        const {name, placeholder, value, label, hidden, tooltip} = this.props;
        return (
            <div className={`card-content-item ${hidden && "is-hidden"}`} id={name.toLowerCase()} data-tip={tooltip}>
                <label htmlFor={name}>{label}</label>
                <input
                    className="input"
                    name={name}
                    type="text"
                    placeholder={placeholder}
                    value={value}
                    onChange={({target}) => this.props.onChange(target.value)}
                />
            </div>
        );
    }
}

TextInput.propTypes = {
    name: PropTypes.string.isRequired,
    placeholder: PropTypes.string,
    value: PropTypes.string.isRequired,
    label: PropTypes.string.isRequired,
    hidden: PropTypes.bool.isRequired,
    tooltip: PropTypes.string,
    onChange: PropTypes.func.isRequired,
};

export default TextInput;
