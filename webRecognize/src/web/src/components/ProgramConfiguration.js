import React from "react";
import PropTypes from "prop-types";

// the render prop
import {Translation} from "react-i18next";

import ConfigurationGroup from "./ConfigurationGroup";

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
                <div className="card-content program-config-content">
                    {this.props.elements.groups.map((group) => (
                        <ConfigurationGroup
                            key={group.name}
                            name={group.name}
                            group={group}
                            values={this.props.programConfiguration}
                            onChangeValue={(target, value) => this.props.changeTargetValue(target, value)}
                        />
                    ))}
                </div>
            </div>
        );
    }
}

ProgramConfiguration.propTypes = {
    programConfiguration: PropTypes.object.isRequired,
    changeTargetValue: PropTypes.func.isRequired,
    elements: PropTypes.object.isRequired,
};

export default ProgramConfiguration;
