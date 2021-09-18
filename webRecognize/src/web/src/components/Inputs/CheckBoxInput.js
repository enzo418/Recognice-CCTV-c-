import React from "react";
import PropTypes from "prop-types";

class TextInput extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            checked: props.checked,
        };
    }

    render() {
        const {name, checked, label, tooltip, hidden} = this.props;
        return (
            <div
                className={`card-content-item ${hidden && "is-hidden"}`}
                id={name.toLowerCase()}
                data-tip={tooltip}
                onClick={() => this.props.onChange(!checked)}>
                <input type="checkbox" name={name} checked={checked} onChange={() => true} />
                <label className="checkbox" htmlFor={name}>
                    {label}
                </label>
            </div>
        );
    }
}

TextInput.propTypes = {
    name: PropTypes.string.isRequired,
    checked: PropTypes.bool.isRequired,
    label: PropTypes.string.isRequired,
    hidden: PropTypes.bool.isRequired,
    tooltip: PropTypes.string,
    onChange: PropTypes.func.isRequired,
};

export default TextInput;
