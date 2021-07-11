import i18n from "i18next";
import {initReactI18next} from "react-i18next";

import Translations from "./translations.json";

for (let key in Translations) {
    Translations[key] = {translations: Translations[key]};
}

i18n.use(initReactI18next).init({
    // we init with resources
    resources: Translations,
    fallbackLng: "en",
    debug: true,

    // have a common namespace used around the full app
    ns: ["translations"],
    defaultNS: "translations",

    keySeparator: false, // we use content as keys

    interpolation: {
        escapeValue: false, // react already safes from xss
        // alwaysFormat: true,
        // format: function(value, format, lng) {
        //     if (format === 'uppercase') return value.toUpperCase();
        //     if(value instanceof Date) return moment(value).format(format);
        //     return value;
        // }
    },

    react: {
        wait: true,
    },

    returnObjects: true,
});

// Wrap t function to avoid case sensitive translation
i18n.f = i18n.t;
i18n.t = function () {
    arguments[0] = arguments[0].toLowerCase();
    return i18n.f.apply(this, arguments);
};

export default i18n;
