import React from "react";
import {Translation} from "react-i18next";

function PageLoader() {
    return (
        <div className="pageloader is-active" id="pageloader">
            <Translation>
                {(t) => (
                    <span className="title" data-translation="Waiting a response from the server">
                        {t("Waiting a response from the server")}
                    </span>
                )}
            </Translation>
        </div>
    );
}
