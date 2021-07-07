import React from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";

function ModalCanvas(props) {
    const {id, cameraIndex, headerElements} = props;
    // const [filename, setFilename] = useState(props.filename);

    function cancel() {
        throw "Method not implemented";
    }

    function accept() {
        throw "Method not implemented";
    }

    return (
        <div className="modal" id={id} data-index={cameraIndex}>
            <div className="modal-background"></div>
            <div className="modal-content container is-fullhd">
                <article className="message is-primary">
                    <div className="message-header">{headerElements}</div>
                    <div className="message-body">
                        <canvas width="640" height="360"></canvas>

                        <div className="buttons-modal-body">
                            <button className="button is-danger" onClick={cancel}>
                                <span className="icon is-small">
                                    <i className="fas fa-times"></i>
                                </span>
                                <span data-translation="Cancel">Cancel</span>
                            </button>

                            <button className="button is-success" onClick={accept}>
                                <span className="icon is-small">
                                    <i className="fas fa-check"></i>
                                </span>
                                <span data-translation="Save">Save</span>
                            </button>
                        </div>
                    </div>
                </article>
            </div>
        </div>
    );
}

ModalCanvas.propTypes = {
    id: PropTypes.string.isRequired,
    cameraIndex: PropTypes.number.isRequired,
    headerElements: PropTypes.object.isRequired,
};
