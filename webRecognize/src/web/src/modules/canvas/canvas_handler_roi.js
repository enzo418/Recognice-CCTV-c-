import React from "react";
import CanvasHandler from "./canvas_handler";
import {getRectangleDimensions} from "./canvas_utils";
import PropTypes from "prop-types";
import ModalCanvas from "../../components/ModalCanvas";

class CanvasRoiHandler extends CanvasHandler {
    /**
     * Handles a canvas element
     * @param {HTMLCanvasElement} canvasContext
     * @param {string} initialValue
     */
    constructor(props, initialValue) {
        super(props);
        this.roi = initialValue;
        this.p1 = {x: 0, y: 0}; // lt or rb
        this.p2 = {x: 0, y: 0}; // rb or lt

        this.handlers = {
            onMouseMove: this.move.bind(this),
            onTouchMove: this.move.bind(this),
            onMouseDown: this.pressed.bind(this),
            onTouchStart: this.pressed.bind(this),
            onMouseUp: this.release.bind(this),
            onTouchEnd: this.release.bind(this),
        };

        this.header = (
            <p data-translation="Select the camera region of interest">Select the camera region of interest</p>
        );
    }

    componentDidMount() {
        super.componentDidMount();
        this.onReady(this.props.image, this.props.initialValue);

        // console.log(this.canvas);
        // console.log("context:", this.ctx);
        this.updateCanvasPosition();
    }

    getValue() {
        return this.roi;
    }

    setROI(roi) {
        this.roi = roi;
    }

    move(e) {
        e.preventDefault();
        e = (e.touches || [])[0] || e;
        if (this.clickPressed) {
            var image = new Image();
            image.onload = () => {
                this.ctx.drawImage(image, 0, 0);

                const x1 = e.clientX - this.x;
                const y1 = e.clientY - this.y;

                this.p2 = {x: x1, y: y1};

                const x0 = this.p1.x;
                const y0 = this.p1.y;
                const width = x1 - x0;
                const heigth = y1 - y0;

                this.ctx.strokeRect(x0, y0, width, heigth);
            };

            image.src = "data:image/jpg;base64," + this.lastImage;
        }
    }

    // Click or touch pressed
    pressed(e) {
        e.preventDefault();
        e = (e.touches || [])[0] || e;
        this.clickPressed = true;
        const x = e.clientX - this.x;
        const y = e.clientY - this.y;

        this.p1 = {x, y};
    }

    // Click or touch released
    release(e) {
        this.clickPressed = false;

        const [lt, width, heigth] = getRectangleDimensions(this.p1, this.p2);

        this.roi = `[${lt.x},${lt.y}],[${width}, ${heigth}]`;
        e.preventDefault();
    }

    /**
     * Callback to update the image displayed in the canvas
     * @param {strng} frame base64 encoded image
     */
    onReady(frame, initialValue) {
        this.roi = initialValue || "";

        let image = new Image();
        image.onload = () => {
            this.ctx.drawImage(image, 0, 0);

            this.ctx.strokeStyle = "Red";
            this.ctx.lineWidth = 5;

            if (this.roi.length > 0) {
                var roi = this.stringToRoi(this.roi);
                if (roi.length > 0) {
                    this.ctx.strokeRect(roi[0], roi[1], roi[2], roi[3]);
                }
            }

            this.updateCanvasPosition();
        };

        image.src = "data:image/jpg;base64," + frame;

        this.lastImage = frame;
    }

    render() {
        return (
            <ModalCanvas header={this.header} onAccept={this.props.onAccept} onCancel={this.props.onCancel}>
                <canvas ref={this.canvas} {...this.handlers} width="640" height="360" />
            </ModalCanvas>
        );
    }
}

CanvasRoiHandler.propTypes = {
    image: PropTypes.string.isRequired,
    initialValue: PropTypes.string,
    callbackOnMounted: PropTypes.func,
    onCancel: PropTypes.func.isRequired,
    onAccept: PropTypes.func.isRequired,
};

export default CanvasRoiHandler;
