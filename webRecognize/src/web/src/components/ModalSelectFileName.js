import React, {useState} from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";

import Modal from "./Modal";

function ModalSelectFileName(props) {
    const [filename, setFilename] = useState(props.filename);

    let header = <Translation>{(t) => <p>{t("Enter the filename")}</p>}</Translation>;
    let body = (
        <div>
            {" "}
            <Translation>
                {(t) => <label htmlFor="file-name">{t("Insert the name of the new file (without extension)")}</label>}
            </Translation>
            <div className="file-name-container">
                <span className="root-dir-configurations"></span>
                <input
                    className="input"
                    type="text"
                    name="file-name"
                    value={filename}
                    onChange={({target}) => setFilename(target.value)}
                />
                <span>.ini</span>
            </div>
        </div>
    );

    let footer = (
        <div className="buttons-config-selector">
            <button className="button is-success" onClick={() => props.callback({cancelled: true})}>
                <span className="icon is-small">
                    <i className="fas fa-check"></i>
                </span>
                <Translation>{(t) => <span>{t("Cancel")}</span>}</Translation>
            </button>

            <button
                className="button is-success"
                disabled={filename === ""}
                onClick={() => props.callback({cancelled: false, value: filename})}>
                <span className="icon is-small">
                    <i className="fas fa-check"></i>
                </span>
                <span>Ok</span>
            </button>
        </div>
    );

    return <Modal className="modal-file" header={header} body={body} footer={footer} />;
}

ModalSelectFileName.propTypes = {
    filename: PropTypes.string.isRequired,
    callback: PropTypes.func.isRequired,
};

export default ModalSelectFileName;
