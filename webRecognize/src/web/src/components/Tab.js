import React from 'react';
import PropTypes from 'prop-types';

function Tab(props) {
    const { cameraIndex, cameraName } = props;
    return (
        <li data-config={`camera-${cameraIndex}`} id={`tab-camera-${cameraIndex}`}>
            <a>
                <span>{cameraName}</span>
            </a>
        </li>
    );
}

Tab.propTypes = {
    cameraIndex: PropTypes.number.isRequired,
    cameraName: PropTypes.string.isRequired,
};
