import React from 'react';
import PropTypes from 'prop-types';

class TextInput extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            checked: props.checked,
        };
    }

    render() {
        const { name, checked, label, tooltip, hidden } = this.props;
        return (
            <div className={`card-content-item ${hidden && 'is-hidden'}`} id={name.toLowerCase()} data-tip={tooltip}>
                <label className="checkbox">
                    <input type="checkbox" name={name} checked={checked}>
                        {label}
                    </input>
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
    tooltip: PropTypes.string.isRequired,
};
