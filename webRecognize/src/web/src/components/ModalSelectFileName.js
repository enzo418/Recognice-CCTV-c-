import React, {useState} from "react";
import PropTypes from "prop-types";
import {Translation} from "react-i18next";

function ModalSelectFileName(props) {
    const [filename, setFilename] = useState(props.filename);

    function cancel() {
        throw "Method not implemented";
    }

    function accept() {
        throw "Method not implemented";
    }

    return (
        <div className="modal" id="modal-file-name">
            <div className="modal-background"></div>
            <div className="modal-content container is-fullhd">
                <article className="message is-primary">
                    <div className="message-header">
                        <Translation>{(t) => <p>{t("Select the name of the file")}</p>}</Translation>
                    </div>
                    <div className="message-body">
                        <Translation>
                            {(t) => (
                                <label htmlFor="file-name">
                                    {t("Insert the name of the new file (without extension)")}
                                </label>
                            )}
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

                        <div className="buttons-config-selector">
                            <button className="button is-success" onClick={cancel}>
                                <span className="icon is-small">
                                    <i className="fas fa-check"></i>
                                </span>
                                <Translation>{(t) => <span>{t("Cancel")}</span>}</Translation>
                            </button>

                            <button className="button is-success" onClick={accept}>
                                <span className="icon is-small">
                                    <i className="fas fa-check"></i>
                                </span>
                                <span>Ok</span>
                            </button>
                        </div>
                    </div>
                </article>
            </div>
        </div>
    );
}

ModalSelectFileName.propTypes = {
    filename: PropTypes.string.isRequired,
};
