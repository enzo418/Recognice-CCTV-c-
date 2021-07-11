import React from "react";
import PropTypes from "prop-types";

function Modal(props) {
    return (
        <div className="modal is-active is-dark">
            <div className="modal-background"></div>
            <div className="modal-card">
                <header className="modal-card-head">{props.header}</header>
                <section className="modal-card-body">{props.body}</section>
                <footer className="modal-card-foot">{props.footer}</footer>
            </div>
        </div>
    );
}

Modal.propTypes = {
    header: PropTypes.any,
    body: PropTypes.any,
    footer: PropTypes.any,
};

export default Modal;
