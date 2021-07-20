import React from "react";
import PropTypes from "prop-types";

function Notification(props) {
    const {type, content, moment_date} = props.notification;
    console.log(props.notification);
    let media;

    if (type === "image") {
        media = (
            <div className="image">
                <img src={content} data-src={content} alt="Image" />
            </div>
        );
    } else if (type === "text") {
        media = <h3 className="subtitle is-3">{content}</h3>;
    } else if (type === "video") {
        media = <video width="640" height="360" preload="metadata" src={content} controls autoPlay="true" data-videosrc={content}></video>;
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
    notification: PropTypes.object.isRequired,
};

export default Notification;
