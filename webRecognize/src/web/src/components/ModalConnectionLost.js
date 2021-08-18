import React, {useState} from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";

import Modal from "./Modal";

function ModalConnectionLost(props) {
    let header = <Translation>{(t) => <p>{t("the connection to the server has been lost")}</p>}</Translation>;
    let body = (
        <div>
            <Translation>{(t) => <span>{t("verify that the server is running to diagnose the problem")}</span>}</Translation>
        </div>
    );

    let footer = (
        <div className="buttons-config-selector">
            <button
                className="button is-success"
                onClick={() => props.closeModal()}>
                <span className="icon is-small">
                    <i className="fas fa-check"></i>
                </span>
                <span>Ok</span>
            </button>
        </div>
    );

    return <Modal className="modal-file" header={header} body={body} footer={footer} />;
}

ModalConnectionLost.propTypes = {
    closeModal: PropTypes.func.isRequired
};

export default ModalConnectionLost;
