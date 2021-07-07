import React from "react";
// import PropTypes from 'prop-types';

// the render prop
import {Translation} from "react-i18next";

class ProgramConfiguration extends React.Component {
    constructor(props) {
        super(props);
        this.state = {};
    }

    render() {
        return (
            <div className="card" id="program">
                <header className="card-header">
                    <Translation>
                        {(t) => <p className="card-header-title">{t("Program configuration")}</p>}
                    </Translation>
                </header>
                <div className="card-content program-config-content"></div>
            </div>
        );
    }
}

ProgramConfiguration.propTypes = {};
