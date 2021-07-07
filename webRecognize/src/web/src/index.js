import { React, ReactDOM } from 'react';

import { I18nextProvider } from 'react-i18next';
import i18n from './i18n';

// wrap the app in I18next Provider with the configuration loaded from i18n.js

ReactDOM.render(
    <I18nextProvider i18n={i18n}>
        <App />
    </I18nextProvider>,
    document.getElementById('app')
);
