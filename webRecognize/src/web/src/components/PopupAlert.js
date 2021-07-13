import React, {useState} from "react";
import PropTypes from "prop-types";
import {withTranslation} from "react-i18next";

const deleteAlert = (ev) => ev.target.parentNode.remove();

function PopupAlert(props) {
    const {alert} = props;
    const [showExtra, setExtraVisible] = useState(true);

    return (
        <div
            onMouseLeave={() => props.setLifeAlert(props.id, false)}
            onMouseEnter={() => props.setLifeAlert(props.id, true)}
            className={"notification " + (alert.status === "ok" ? "is-success" : "is-danger")}>
            <button className="delete" onClick={deleteAlert}></button>
            <span className="notification-text">{props.t(alert.message)}</span>
            <p>
                <a hidden={!showExtra} onClick={() => setExtraVisible(!showExtra)}>
                    {props.t("show more")}
                </a>
            </p>
            <span className="notification-extra-text" hidden={showExtra}>
                {alert.extra}
            </span>
        </div>
    );
}

PopupAlert.propTypes = {
    id: PropTypes.number.isRequired,
    alert: PropTypes.object.isRequired,
    setLifeAlert: PropTypes.func.isRequired,
    t: PropTypes.any,
};

export default withTranslation()(PopupAlert);
