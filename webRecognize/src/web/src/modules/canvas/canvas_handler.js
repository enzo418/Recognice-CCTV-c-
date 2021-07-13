import React from "react";

// abstract class that adds functionality to a canvas
class CanvasHandler extends React.Component {
    constructor(props) {
        super(props);
        this.lastImage = "";

        this.x = 0; // canvas position in x
        this.y = 0; // canvas position in y (with scrollbar)

        this.clickPressed = false;

        this.canvas = React.createRef();

        // canvas envent handlers
        this.handlers = {};

        this.header = null;

        this.state = {
            size: {
                width: 640,
                height: 360,
            },
        };
    }

    getHeaders() {
        return this.header;
    }

    // get the value
    getValue() {
        return "";
    }

    setImage(image) {
        this.lastImage = image;
    }

    componentDidMount() {
        console.log(this.canvas);
        this.ctx = this.canvas.current.getContext("2d");
        console.log("context:", this.ctx);
        this.updateCanvasPosition();
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

    render() {
        return (
            <canvas
                ref={this.canvas}
                {...this.handlers}
                width={this.state.size.width + "px"}
                height={this.state.size.height + "px"}
            />
        );
    }
}

export default CanvasHandler;
