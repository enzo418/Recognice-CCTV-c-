import React from "react";
import simplify from "simplify-js";

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

    updateCanvasPosition() {
        var bounds = this.canvas.current.getBoundingClientRect();
        this.x = bounds.left;
        this.y = bounds.top;
    }

    render() {
        return <canvas ref={this.canvas} {...this.handlers} width="640" height="360" />;
    }
}

export default CanvasHandler;
