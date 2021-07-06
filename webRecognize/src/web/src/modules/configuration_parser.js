function getHeaders (str) {
    var re = /(PROGRAM|CAMERA)/g;
    var headers_match = []
    var match;
    while ((match = re.exec(str)) != null) {
        var start = match.index - 2;
        var end = match.index + match[0].length + 2;
        var name = match[0]
        headers_match.push({ start, end, name });
    }
    return headers_match;
}

export default function parseConfiguration(str) {
    var headers = { "program": {}, "cameras": [] };

    var headers_match = getHeaders(str);        
    for (var i = 0; i < headers_match.length; i++) {
        var nxt = headers_match[i + 1] || []

        var cam_str = str.slice(headers_match[i]["end"], nxt["start"] || str.length);

        var obj = {};
        var lines = cam_str.split('\n');
        for (var j = 0; j < lines.length; j++) {
            if (lines[j].length > 0) {
                var eq = lines[j].indexOf("=");
                var id = lines[j].slice(0, eq).toLowerCase();
                var val = lines[j].slice(eq + 1, lines[j].length);
                obj[id] = val;
            }
        }

        if (headers_match[i]["name"] == "CAMERA")
            headers["cameras"].push(obj);
        else
            headers["program"] = obj;
    }

    return headers;
}