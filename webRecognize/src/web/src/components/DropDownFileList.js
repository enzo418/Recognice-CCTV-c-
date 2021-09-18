import React, {useState} from "react";
import PropTypes from "prop-types";

function DropDownFileList(props) {
    const [displayed, setDisplayed] = useState(false);
    return (
        <div className={"dropdown dropdown-file " + (displayed ? " is-active" : "")}>
            <div className="dropdown-trigger">
                <button
                    className="button is-dark"
                    aria-haspopup="true"
                    aria-controls="dropdown-menu"
                    onClick={() => setDisplayed(!displayed)}>
                    <span className="icon is-small">
                        <i className="fas fa-file-alt"></i>
                    </span>
                    <span>{props.currentFile}</span>
                    <span className="icon is-small">
                        <i className="fas fa-angle-down" aria-hidden="true"></i>
                    </span>
                </button>
            </div>
            <div className="dropdown-menu" id="dropdown-menu" role="menu">
                <div className="dropdown-content">
                    {props.files.map((file) => (
                        <div
                            key={file}
                            className="dropdown-item"
                            onClick={() => {
                                setDisplayed(!displayed);
                                props.changeFile(file);
                            }}>
                            {file}
                        </div>
                    ))}
                </div>
            </div>
        </div>
    );
}

DropDownFileList.propTypes = {
    files: PropTypes.arrayOf(PropTypes.string).isRequired,
    changeFile: PropTypes.func.isRequired,
    currentFile: PropTypes.string.isRequired,
};

export default DropDownFileList;
