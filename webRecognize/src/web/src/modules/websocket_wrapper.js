/**
 * Wrappers the WebSocket default implementation
 * to enable the user to easily subscribe to custom
 * event (id) sended from our server.
 *
 * Since our server sends the message with the format
 * {key: content}
 * this wrapper just calls all the subscribers to key.
 *
 * There are two default id that are not treated as
 * messages: 'open' and 'close'.
 */
export class WrapperWebSocket {
    constructor(url) {
        this.m_socket = new WebSocket(url);
        this.m_messageHandlerRegistered = false;
        this.m_openHandlerRegistered = false;
        this.m_closeHandlerRegistered = false;
        this.m_handlers = {open: [], close: []};
    }

    on(id, callback) {
        switch (id) {
            case "close":
                this.__registerCloseEventHandler(callback);
                break;
            case "open":
                this.__registerOpenEventHandler(callback);
                break;
            default:
                this.__registerMessageEventHandler(id, callback);
                break;
        }
    }

    __onmessage(ev) {
        let ev_data = ev.data;
        let message;

        try {
            message = JSON.parse(ev_data);
        } catch (error) {
            throw new Error("Couldn't parse the websocket data event.");
        }

        let keys = Object.keys(message);

        if (keys.length === 1) {
            let id = keys[0];
            let content = message[id];

            // call the handlers
            for (var i in this.m_handlers[id]) {
                this.m_handlers[id][i](content);
            }
        } else {
            console.warn("WARNING: couldn't handle event, probably wrong format sended from server.");
        }
    }

    __onclose(ev) {
        for (var i in this.m_handlers["close"]) {
            this.m_handlers["close"][i](ev);
        }
    }

    __onopen(ev) {
        for (var i in this.m_handlers["open"]) {
            this.m_handlers["open"][i](ev);
        }
    }

    __registerMessageEventHandler(id, callback) {
        if (!this.m_messageHandlerRegistered) {
            this.m_socket.addEventListener("message", this.__onmessage.bind(this));
            this.m_messageHandlerRegistered = true;
        }

        if (id in this.m_handlers) {
            this.m_handlers[id].push(callback);
        } else {
            this.m_handlers[id] = [callback];
        }
    }

    __registerCloseEventHandler(callback) {
        if (!this.m_closeHandlerRegistered) {
            this.m_socket.addEventListener("close", this.__onclose.bind(this));
            this.m_closeHandlerRegistered = true;
        }

        this.m_handlers.close.push(callback);
    }

    __registerOpenEventHandler(callback) {
        if (!this.m_openHandlerRegistered) {
            this.m_socket.addEventListener("open", this.__onopen.bind(this));
            this.m_openHandlerRegistered = true;
        }

        this.m_handlers.open.push(callback);
    }
}