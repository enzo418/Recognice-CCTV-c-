
class ConfigurationFile {
    constructor() { }

    SetConfiguration(config) {
        this._cfg = config;
    }

    GetConfiguration() {
        return this._cfg;
    }

    Save() {
        console.log("Save");
        throw new Error("Not implemented");
    }
}

export default ConfigurationFile;