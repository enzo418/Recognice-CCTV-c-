import React from "react";
import CanvasHandler from "./canvas_handler";
import {getRandom, getRectangleDimensions} from "./canvas_utils";
import PropTypes from "prop-types";
import ModalCanvas from "../../components/ModalCanvas";

class CanvasAreasHandler extends CanvasHandler {
    constructor(props) {
        super(props);
        this.areas = []; // area: {lt: {x, y}, width, height}
        this.current = {
            p1: {x: 0, y: 0},
            p2: {x: 0, y: 0},
            color: "",
        };
        this.areasString = "";
        this.lastClick = null;

        this.colors = [
            "Aqua",
            "Red",
            "Blue",
            "Chartreuse",
            "Crimson",
            "Cyan",
            "DeepPink",
            "Gold",
            "LawnGreen",
            "PaleTurquoise",
        ];

        this.handlers = {
            onMouseMove: this.move.bind(this),
            onTouchMove: this.move.bind(this),
            onMouseDown: this.pressed.bind(this),
            onTouchStart: this.pressed.bind(this),
            onMouseUp: this.release.bind(this),
            onTouchEnd: this.release.bind(this),
        };

        this.header = (
            <div className="ignored-areas-header">
                <p data-translation="Select the ignored areas of the camera">Select the ignored areas of the camera</p>
                <button id="remove-all-areas" className="button" data-translation="Remove all">
                    Remove all
                </button>
            </div>
        );
    }

    getValue() {
        return this.areasString;
    }

    componentDidMount() {
        super.componentDidMount();
        this.onReady(this.props.image, this.props.initialValue);
    }

    componentDidUpdate() {
        console.log("updated");
    }

    removeAll() {
        this.areas = [];
        this.areasString = "";

        var image = new Image();
        image.onload = () => this.ctx.drawImage(image, 0, 0);

        image.src = "data:image/jpg;base64," + this.lastImage;
    }

    // Mouse or touch moved
    move(e) {
        e.preventDefault();
        e = (e.touches || [])[0] || e;
        if (this.clickPressed) {
            var image = new Image();
            image.onload = () => {
                // check again... there is alot of this calls at the same time and causes problems
                if (this.clickPressed) {
                    this.ctx.drawImage(image, 0, 0);

                    this.areas.forEach((area) => {
                        this.ctx.strokeStyle = area.color;
                        this.ctx.strokeRect(area.lt.x, area.lt.y, area.width, area.heigth);
                    });

                    const x1 = e.clientX - this.x;
                    const y1 = e.clientY - this.y;

                    this.current.p2 = {x: x1, y: y1};

                    const x0 = this.current.p1.x;
                    const y0 = this.current.p1.y;
                    const width = x1 - x0;
                    const heigth = y1 - y0;

                    this.ctx.strokeStyle = this.current.color;

                    this.ctx.strokeRect(x0, y0, width, heigth);
                }
            };

            image.src = "data:image/jpg;base64," + this.lastImage;
        }
    }

    // Click or touch pressed
    pressed(e) {
        e.preventDefault();
        e = (e.touches || [])[0] || e;
        this.lastClick = performance.now();
        this.clickPressed = true;
        const x = e.clientX - this.x;
        const y = e.clientY - this.y;

        this.current.color = this.colors[getRandom(0, this.colors.length)];

        this.current.p1 = this.current.p2 = {x, y};
    }

    // Click or touch released
    release(e) {
        this.clickPressed = false;
        var time = (performance.now() - this.lastClick) / 1000;

        const [lt, width, heigth] = getRectangleDimensions(this.current.p1, this.current.p2);

        // if it's dimesion is small enough and was a quick click then delete it, else save it

        if (time < 2 && time > 0 && width > -2 && width < 2 && heigth > -2 && heigth < 2) {
            const p = this.current.p1;
            this.areas.forEach((area, index, object) => {
                if (
                    p.x >= area.lt.x &&
                    p.x <= area.lt.x + area.width &&
                    p.y >= area.lt.y &&
                    p.y <= area.lt.y + area.heigth
                ) {
                    object.splice(index, 1);
                }
            });

            // draw areas
            var image = new Image();
            image.onload = () => {
                this.ctx.drawImage(image, 0, 0);

                this.areas.forEach((area) => {
                    this.ctx.strokeStyle = area.color;
                    this.ctx.strokeRect(area.lt.x, area.lt.y, area.width, area.heigth);
                });
            };

            image.src = "data:image/jpg;base64," + this.lastImage;
        } else {
            this.areas.push({lt, width, heigth, color: this.current.color});

            this.areasString = "";
            this.areas.forEach((area) => {
                this.areasString += `[${area.lt.x},${area.lt.y}],[${area.width}, ${area.heigth}],`;
            });

            this.areasString = this.areasString.substring(0, this.areasString.length - 1);

            this.current.p1 = {x: 0, y: 0};
            this.current.p2 = {x: 0, y: 0};
        }
        e.preventDefault();
    }

    /**
     * Callback to update the image displayed in the canvas
     * @param {string} frame base64 encoded image
     * @param {string} initialValue initial value
     */
    onReady(frame, initialValue) {
        this.areasString = initialValue || "";
        this.areas = [];

        let image = new Image();
        image.onload = () => {
            this.ctx.drawImage(image, 0, 0);

            this.ctx.strokeStyle = "Red";
            this.ctx.lineWidth = 5;

            if (this.areasString.length > 0) {
                var numbers = this.areasString.match(/\d+/g).map((i) => parseInt(i));
                if (numbers.length % 4 === 0) {
                    for (var base = 0; base < numbers.length; base += 4) {
                        const color = this.colors[getRandom(0, this.colors.length)];
                        const lt = {x: numbers[base + 0], y: numbers[base + 1]},
                            width = numbers[base + 2],
                            heigth = numbers[base + 3];

                        this.areas.push({lt, width, heigth, color});

                        this.ctx.strokeStyle = color;
                        this.ctx.strokeRect(lt.x, lt.y, width, heigth);
                    }
                }
            }

            console.log("areas loaded: ", this.areas.length);
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

CanvasAreasHandler.propTypes = {
    image: PropTypes.string.isRequired,
    initialValue: PropTypes.string,
    callbackOnMounted: PropTypes.func,
    onCancel: PropTypes.func.isRequired,
    onAccept: PropTypes.func.isRequired,
};

export default CanvasAreasHandler;
