import React from "react";
import simplify from "simplify-js";
import CanvasHandler from "./canvas_handler";
import {getRandom, getRectangleDimensions} from "./canvas_utils";

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
    }

    removeAll() {
        this.areas = [];
        this.areasString = "";
        var camindex = $("#modal-igarea").data("index");
        document.querySelector("#camera-" + camindex).querySelector('input[name="ignoredareas"]').value = "";

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
            image.onload = function () {
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
            image.onload = function () {
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
}

export default CanvasAreasHandler;
