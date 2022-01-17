import React from "react";
import PropTypes from "prop-types";
import Modal from "./Modal";

function ModalCanvas(props) {
    return (
        <Modal
            className={" " + props.className}
            header={props.header}
            body={
                // className="message-body"
                <div className="">
                    {props.children}

                    <div className="buttons-modal-body">
                        <button className="button is-danger" onClick={() => props.onCancel()}>
                            <span className="icon is-small">
                                <i className="fas fa-times"></i>
                            </span>
                            <span data-translation="Cancel">Cancel</span>
                        </button>

                        <button className="button is-success" onClick={() => props.onAccept()}>
                            <span className="icon is-small">
                                <i className="fas fa-check"></i>
                            </span>
                            <span data-translation="Save">Save</span>
                        </button>
                    </div>
                </div>
            }></Modal>
    );
}

ModalCanvas.propTypes = {
    className: PropTypes.string,
    header: PropTypes.any,
    onAccept: PropTypes.func.isRequired,
    onCancel: PropTypes.func.isRequired,
    children: PropTypes.any,
};

export default ModalCanvas;
