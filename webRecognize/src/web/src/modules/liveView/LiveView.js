import React from "react";
import PropTypes from "prop-types";

class LiveView extends React.Component {
    constructor(props) {
        super(props);

        this.GetAndSetLiveView = this.GetAndSetLiveView.bind(this);
        this.image_video = React.createRef();
    }

    onImageLoaded(e) {
        // Moz.org: Browsers will release object URLs automatically
        // when the document is unloaded; however, for optimal
        // performance and memory usage, if there are safe times
        // when you can explicitly unload them, you should do so.
        URL.revokeObjectURL(this.image_video.current.src);

        this.props.onLoad(e);
    }

    componentDidMount() {
        this.GetAndSetLiveView(this.props.feed_id);
    }

    GetAndSetLiveView(feed) {
        const socket = new WebSocket(`ws://${window.location.host}${feed}`);

        // set socket as binary
        socket.binaryType = "blob";

        // Listen for frames
        socket.addEventListener("message", (event) => {
            // Blob: https://developer.mozilla.org/en-US/docs/Web/API/Blob#creating_a_url_representing_the_contents_of_a_typed_array
            // Blob to file: https://developer.mozilla.org/en-US/docs/Web/API/File/Using_files_from_web_applications#example_using_object_urls_to_display_images

            // console.log(event.data);
            const blob = new Blob([event.data], { type: 'image/jpeg' });

            this.image_video.current.src = URL.createObjectURL(blob);
        });

        return socket;
    }

    render() {
        return <img ref={this.image_video} onLoad={this.onImageLoaded.bind(this)}></img>;
    }
}

LiveView.propTypes = {
    feed_id: PropTypes.string.isRequired,
    onLoad: PropTypes.func.isRequired,
};

export default LiveView;
