import React from "react";

// abstract class that adds functionality to a canvas
class CanvasHandler extends React.Component {
    constructor(props) {
        super(props);
        this.lastImage = "";

        this.x = 0; // canvas position in x
        this.y = 0; // canvas position in y (with scrollbar)

        this.clickPressed = false;

        // canvas envent handlers
        this.handlers = {};

        this.headers = null;

        this.state = {
            size: {
                width: 640,
                height: 360,
            },
        };

        this.canvas = React.createRef();
    }

    getHeaders() {
        return this.headers;
    }

    // get the value
    getValue() {
        return "";
    }

    setImage(image) {
        this.lastImage = image;
    }

    componentDidMount() {
        this.ctx = this.canvas.current.getContext("2d");
    }

    setCanvasSize({width, height}) {
        this.setState((prev) => {
            if (width) {
                prev.size.width = width;
            }

            if (height) {
                prev.size.height = height;
            }
        });
    }

    repaintCanvas(callbackOnImageLoaded) {
        var image = new Image();
        image.onload = () => callbackOnImageLoaded();
        image.src = "data:image/jpg;base64," + this.lastImage;
    }

    updateCanvasPosition() {
        var bounds = this.canvas.current.getBoundingClientRect();
        this.x = bounds.left;
        this.y = bounds.top;
    }

    /*
    render() {
        return ;
    }*/
}

export default CanvasHandler;
