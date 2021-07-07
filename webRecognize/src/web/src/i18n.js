import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';

import Translations from './translations.json';

for (let key in Translations) {
    Translations[key] = { translations: Translations[key] };
}

i18n.use(initReactI18next).init({
    // we init with resources
    resources: Translations,
    fallbackLng: 'en',
    // debug: true,

    // have a common namespace used around the full app
    ns: ['translations'],
    defaultNS: 'translations',

    keySeparator: false, // we use content as keys

    interpolation: {
        escapeValue: false, // react already safes from xss
    },

    react: {
        wait: true,
    },
});

export default i18n;
