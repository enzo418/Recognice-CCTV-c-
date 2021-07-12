const headers = {
    camera: "[CAMERA]",
    program: "[PROGRAM]",
};

function searchElement(key, elements) {
    let res;
    if ("elements" in elements) {
        res = elements.elements.find((el) => el.target.toLowerCase() === key);
    }

    if (!res && "groups" in elements) {
        for (var i = 0; i < elements.groups.length; i++) {
            res = searchElement(key, elements.groups[i]);
            if (res) {
                break;
            }
        }
    }

    return res;
}

const parseValue = (val, type) => {
    try {
        return type === "number" ? parseFloat(val) : type === "boolean" ? (val === "1" ? true : false) : val;
    } catch (ex) {
        console.error("Error parsing value: ", {val, type});
    }
};

function getHeaders(str) {
    var re = /\n\[(PROGRAM|CAMERA)\]/g;
    var headers_match = [];
    var match;
    while ((match = re.exec(str)) !== null) {
        var start = match.index;
        var end = match.index + match[0].length;
        var name = match[1];
        headers_match.push({start, end, name});
    }
    return headers_match;
}

function parseConfiguration(str, elements) {
    var headers = {program: {}, cameras: []};

    var headers_match = getHeaders(str);
    for (var i = 0; i < headers_match.length; i++) {
        var nxt = headers_match[i + 1] || [];

        var cam_str = str.slice(headers_match[i]["end"], nxt["start"] || str.length);

        var obj = {};
        var lines = cam_str.split("\n");
        for (var j = 0; j < lines.length; j++) {
            if (lines[j].length > 0 && lines[j][0] !== ";" && lines[j][0] !== "#") {
                var eq = lines[j].indexOf("=");
                var id = lines[j].slice(0, eq).toLowerCase();
                var val = lines[j].slice(eq + 1, lines[j].length);
                let type = searchElement(id, elements[headers_match[i]["name"].toLowerCase()]).type;
                obj[id] = parseValue(val, type);
                // console.log({cfg: headers_match[i]["name"].toLowerCase(), id, type, value: val, parsed: obj[id]});
            }
        }

        if (headers_match[i]["name"] === "CAMERA") headers["cameras"].push(obj);
        else headers["program"] = obj;
    }

    return headers;
}

function configurationToString(cfg) {
    return Object.entries(cfg).reduce((ac, current) => {
        const [key, value] = current;
        if (key !== "id") {
            ac += key + "=" + value + "\n";
        }
        return ac;
    }, "");
}

function configurationsToString(configurations) {
    let program = headers.program + "\n" + configurationToString(configurations.program);
    let cameras =
        headers.camera +
        "\n" +
        configurations.cameras.map((camera) => configurationToString(camera)).join("\n[CAMERA]\n");

    return program + cameras;
}

export default {
    parseConfiguration,
    configurationsToString,
};
