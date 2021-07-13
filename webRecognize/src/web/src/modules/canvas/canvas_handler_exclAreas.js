import React from "react";
import simplify from "simplify-js";
import CanvasHandler from "./canvas_handler";
import {pointPolygonTest} from "./canvas_utils";

class CanvasExclusivityAreasHandler extends CanvasHandler {
    constructor(props) {
        super(props);

        this.areas = []; // area: {type: (allow|deny), points: []}
        this.current = {
            points: [],
            aproximated: false,
        };
        this.areasString = "";
        this.colors = {
            allow: "Chartreuse",
            deny: "Crimson",
        };
        this.lastUndoEvents = []; // array of { type: string [point or area], obj: Object }

        this.select = {
            areaSelectedIndex: null,
            selectModeActive: false,
        };

        this.header = (
            <div className="message-header">
                <p data-translation="Select the exclusivity areas of the camera">
                    Select the exclusivity areas of the camera
                </p>

                <div className="message-header-button">
                    <button id="button-close-poly" className="button sizable" data-translation="Close polygon">
                        Close polygon
                    </button>
                    <button
                        id="button-aprox-poly"
                        className="button sizable"
                        data-translation="Aproximate polygon curve(s)">
                        Aproximate polygon curve(s)
                    </button>

                    <div id="toggle-exclusivity-area-type" className="buttons has-addons selection">
                        <button className="button" data-type="allow">
                            Allow points inside this poly
                        </button>
                        <button className="button is-warning is-selected" data-type="deny">
                            Deny points inside this poly
                        </button>
                    </div>

                    <div className="undo-redo sizable">
                        <button id="button-undo" className="button">
                            <i className="fas fa-undo"></i>
                            <p>Undo</p>
                        </button>
                        <button id="" className="button">
                            <p>Redo</p>
                            <i className="fas fa-redo"></i>
                        </button>
                    </div>

                    <button id="button-remove-all-exclareas" className="button sizable" data-translation="Remove all">
                        Remove all
                    </button>

                    <button
                        id="button-remove-selected-exclareas"
                        className="button sizable selection is-hidden"
                        data-translation="Remove this area">
                        Remove this area
                    </button>

                    <button
                        id="button-start-selected-exclareas"
                        className="button sizable"
                        data-translation="Select area">
                        Select area
                    </button>

                    <button
                        id="button-exit-selected-exclareas"
                        className="button sizable selection is-hidden"
                        data-translation="Exit from selectoin mode">
                        Exit from selectoin mode
                    </button>
                </div>
            </div>
        );

        this.handlers = {
            onMouseMove: () => true,
            onTouchMove: () => true,
            onMouseDown: this.pressed.bind(this),
            onTouchStart: this.pressed.bind(this),
            onMouseUp: () => true,
            onTouchEnd: () => true,
        };
    }

    startSelectMode() {
        this.select.selectModeActive = true;
    }

    onSelect() {
        [...document.querySelector(`${this.selectors.modal} ${this.selectors.headerButtons}`).children].forEach(
            (el) => {
                if (el.classList.contains("selection")) {
                    el.classList.remove("is-hidden");
                } else {
                    el.classList.add("is-hidden");
                }
            }
        );

        // set selection type to the same as the area
        if (this.getTypeSelected() !== this.areas[this.select.areaSelectedIndex].type) {
            $(this.selectors.typeSelectorContainer + " .button").toggleClass("is-selected is-warning");
        }
    }

    removeSelected() {
        this.lastUndoEvents.push({
            type: "areas-removed",
            obj: this.areas.splice(this.select.areaSelectedIndex, 1),
        });
        this.redraw();
        this.select.exitSelectionMode();
    }

    exitSelectionMode() {
        [...document.querySelector(`${this.selectors.modal} ${this.selectors.headerButtons}`).children].forEach(
            (el) => {
                if (
                    el.classList.contains("selection") &&
                    el.id !==
                        this.selectors.typeSelectorContainer.substr(1, this.selectors.typeSelectorContainer.length)
                ) {
                    el.classList.add("is-hidden");
                } else {
                    el.classList.remove("is-hidden");
                }
            }
        );
        this.select.selectModeActive = false;
        this.select.areaSelectedIndex = null;
    }

    removeAll() {
        if (this.areas.length === 0) return;

        this.lastUndoEvents.push({
            type: "areas-removed",
            obj: this.areas.splice(0, this.areas.length),
        });

        this.areasString = "";
        this.current = {
            points: [],
        };

        this.redraw();

        var camindex = $("#modal-exclusivity-areas").data("index");
        document.querySelector("#camera-" + camindex).querySelector('input[name="pointsdiscriminators"]').value = "";
    }

    save(ev, $save) {
        var camindex = $("#modal-exclusivity-areas")[0].dataset["index"];
        var camera = document.querySelector("#camera-" + camindex);

        if ($save) {
            var discriminatorInput = camera.querySelector('input[name="pointsdiscriminators"]');
            this.areas2String();
            discriminatorInput.value = this.areasString;
        }

        this.x = this.y = 0;
        this.areas = [];
        this.areasString = "";
        this.current = {
            points: [],
            color: "",
        };

        camera.querySelector(".button-select-camera-exclusivity-areas").classList.remove("is-loading");
        $("#modal-exclusivity-areas").toggleClass("is-active");
    }

    openModal(ev, cameraIndex) {
        $(ev.target).addClass("is-loading");

        frameRequestOrigin = "exclusivity-areas";

        var cam = document.querySelector(`#camera-${cameraIndex}`);

        var rotation = parseInt(cam.querySelector(`input[name="rotation"]`).value);

        var roi = cam.querySelector('input[name="roi"]').value;

        var url = cam.querySelector(`input[name="url"]`).value || cameras[cameraIndex].url;

        var pointsDiscriminators =
            cam.querySelector(`input[name="pointsdiscriminators"]`).value || cameras[cameraIndex].pointsdiscriminators;

        var parsedRoi = this.stringToRoi(roi);
        if (parsedRoi) {
            $(this.canvas).attr("width", parsedRoi[2]);
            $(this.canvas).attr("height", parsedRoi[3]);
        }

        // update this.areas
        this.string2areas(pointsDiscriminators);

        sendObj("get_camera_frame", {index: cameraIndex, rotation, url, roi});

        unfinishedRequests["get_camera_frame"] = function () {
            setTimeout(function () {
                $(ev.target).removeClass("is-loading");
            }, 500);
        };
    }

    closeCurrentPoly() {
        // Saves the poly drawn into the areas collection
        /// TODO: Check if it's a complex polygon, e.g. has intersection between lines of the poly

        if (this.current.points.length === 0) return;

        if (!this.current.aproximated) {
            if (confirm(_("Aproximate polygon curve(s)? It can increases the program perfomance."))) {
                this.aproxPoly();
            }
        }

        var type = this.getTypeSelected();
        this.areas.push({type: type, points: this.current.points});
        this.current = {
            points: [],
            color: "",
            aproximated: false,
        };
        this.redraw(true);
    }

    aproxPoly() {
        // Aproximates the curves of the poly.
        const epsilon = 5;

        // Simplify curve using https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm,
        // since i'am lazy, i use the optimized version from https://github.com/mourner/simplify-js
        this.current.points = simplify(this.current.points, epsilon, false);

        this.current.aproximated = true;

        this.redraw(true);
    }

    redraw(should_close_path, draw_saved_areas = true) {
        // Draws the canvas with the areas and current points
        var image = new Image();
        image.onload = function () {
            this.ctx.drawImage(image, 0, 0);

            //  Draw the lines that connects the points
            // -----------------------------------------
            this.ctx.beginPath();

            this.ctx.lineWidth = 5;
            this.ctx.strokeStyle = "Green";

            if (this.current.points.length > 1) {
                var first = this.current.points[0];
                this.ctx.moveTo(first.x, first.y);

                for (let index = 1; index < this.current.points.length; index++) {
                    const element = this.current.points[index];

                    this.ctx.lineTo(element.x, element.y);
                }
            }

            if (should_close_path) this.ctx.closePath();

            this.ctx.stroke();

            //  Draw the points
            // -----------------
            this.ctx.beginPath();

            this.ctx.strokeStyle = "Red";
            this.current.points.forEach((point) => {
                this.ctx.moveTo(point.x, point.y);
                this.ctx.arc(point.x, point.y, 3, 0, 2 * Math.PI);
            });

            this.ctx.stroke();

            //  Draw the areas saved
            // ----------------------
            if (draw_saved_areas) {
                this.areas.forEach((area) => {
                    this.ctx.beginPath();
                    this.ctx.strokeStyle = this.colors[area.type];
                    var first = area.points[0];
                    this.ctx.moveTo(first.x, first.y);

                    for (let index = 1; index < area.points.length; index++) {
                        const element = area.points[index];

                        this.ctx.lineTo(element.x, element.y);
                    }

                    this.ctx.closePath();
                    this.ctx.stroke();
                });
            }
        };

        image.src = "data:image/jpg;base64," + this.lastImage;
    }

    redo() {
        // Redo last undo
        const last = this.lastUndoEvents.pop();

        if (last && last.type.length > 0) {
            if (last.type === "point") {
                this.current.points.push(last.obj);
            } else if (last.type === "area") {
                this.areas.push(last.obj);
            } else if (last.type === "areas-removed") this.areas = this.areas.concat(last.obj);
        }

        this.redraw(false);
    }

    undo() {
        // Undo last action
        if (this.current.points.length > 0) {
            this.lastUndoEvents.push({type: "point", obj: this.current.points.pop()});
        } else {
            this.lastUndoEvents.push({type: "area", obj: this.areas.pop()});
        }

        this.redraw(false);
    }

    getTypeSelected() {
        return document.getElementById("toggle-exclusivity-area-type").querySelector(".is-selected").dataset.type;
    }

    areas2String() {
        // Syntax to follow: (allow|deny):ap1_x,ap1_y,...,apn_x,apn_y-(allow|deny):bp1_x,bp1_y,...,bpn_x,bpn_y
        this.areasString = "";
        var isFirst = true;
        this.areas.forEach((area) => {
            this.areasString += (isFirst ? "" : "-") + area.type + ":";

            area.points.forEach((p) => {
                this.areasString += Math.round(p.x) + "," + Math.round(p.y) + ",";
            });

            this.areasString = this.areasString.slice(0, -1);

            isFirst = false;
        });
    }

    /**
     * Parse a string into a array of areas
     * @param {string} pointsDiscriminators poin with the following syntax:
     *  (allow|deny):ap1_x,ap1_y,...,apn_x,apn_y-(allow|deny):bp1_x,bp1_y,...,bpn_x,bpn_y
     */
    string2areas(pointsDiscriminators) {
        if (!pointsDiscriminators || pointsDiscriminators.length === 0) {
            return;
        }

        var currAreaIndex = 0;

        this.areas.push({points: [], color: "", type: ""});
        const r = RegExp("(?<separator>-)?(?:(?<type>allow|deny):)?(?<point_coord>[0-9]+)", "g");
        [...pointsDiscriminators.matchAll(r)].forEach((match) => {
            if (match.groups.separator) {
                // tokens.push(match.groups.separator);
                currAreaIndex++;
                this.areas.push({points: [], color: "", type: ""});
            }

            if (match.groups.type) {
                this.areas[currAreaIndex].type = match.groups.type;
            }

            if (match.groups.point_coord) {
                if (
                    this.areas[currAreaIndex].points.length > 0 &&
                    this.areas[currAreaIndex].points[this.areas[currAreaIndex].points.length - 1].y === null
                ) {
                    this.areas[currAreaIndex].points[this.areas[currAreaIndex].points.length - 1].y =
                        match.groups.point_coord;
                } else {
                    this.areas[currAreaIndex].points.push({x: match.groups.point_coord, y: null});
                }
            }
        });
    }

    onclickbuttonInOut(e) {
        [...this.children].forEach((ch) => ch.classList.remove("is-selected", "is-warning"));
        e.target.classList.add("is-selected", "is-warning");

        if (this.select.selectModeActive) {
            this.areas[this.select.areaSelectedIndex].type = this.getTypeSelected();
            this.redraw();
        }
    }

    // Click or touch pressed
    pressed(e) {
        e.preventDefault();
        e = (e.touches || [])[0] || e; // get touch (mobile) or fall to e (mouse)
        const x = e.clientX - this.x;
        const y = e.clientY - this.y;

        if (!this.select.selectModeActive) {
            this.current.points.push({x, y});

            this.lastUndoEvents = [];

            this.redraw(false);
        } else {
            for (var ia in this.areas) {
                if (pointPolygonTest(this.areas[ia].points, {x, y}) > 0) {
                    this.select.areaSelectedIndex = ia;
                    this.select.onSelect();
                }
            }
        }
    }
}
