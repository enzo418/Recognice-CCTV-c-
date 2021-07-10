import React from "react";
import PropTypes from "prop-types";

function Notification(props) {
    const {type, text, moment_date} = props;
    let media;

    if (type === "image") {
        media = (
            <div className="image">
                <img src="" data-src="${text}" alt="Image" />
            </div>
        );
    } else if (type === "text") {
        media = <h3 className="subtitle is-3">{text}</h3>;
    } else if (type === "video") {
        media = <video width="640" height="360" preload="metadata" src="" controls data-videosrc={text}></video>;
    }

    return (
        <div className={`box ${(type === "text" && "text-notification") || ""}`}>
            {media}
            <h6 className="subtitle is-6 hour" data-date={moment_date}>
                now
            </h6>
        </div>
    );
}

Notification.propTypes = {
    type: PropTypes.string.isRequired,
    text: PropTypes.string.isRequired,
    moment_date: PropTypes.object.isRequired,
};

export default Notification;
