import React from "react";
import PropTypes from "prop-types";

function Modal(props) {
    return (
        <div className={" " + props.className}>
            {props.header}
            {props.body}
            {props.footer}
        </div>
    );
}

Modal.propTypes = {
    header: PropTypes.any,
    body: PropTypes.any,
    footer: PropTypes.any,
    className: PropTypes.string,
};

export default Modal;
