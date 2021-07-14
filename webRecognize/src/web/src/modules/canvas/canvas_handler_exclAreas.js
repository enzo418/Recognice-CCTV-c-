import React from "react";
import simplify from "simplify-js";
import CanvasHandler from "./canvas_handler";
import {pointPolygonTest} from "./canvas_utils";
import PropTypes from "prop-types";
import ModalCanvas from "../../components/ModalCanvas";

const TypeArea = {
    DENY: "deny",
    ALLOW: "allow",
};

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

        // in state only define the variables that the elements use
        this.state = {
            areaSelectedIndex: null,
            selectModeActive: false,
            typeSelected: "allow", // deny|allow
        };

        this.handlers = {
            onMouseMove: () => true,
            onTouchMove: () => true,
            onMouseDown: this.pressed.bind(this),
            onTouchStart: this.pressed.bind(this),
            onMouseUp: () => true,
            onTouchEnd: () => true,
        };

        // this.header = (

        // );

        this.toggleAreaType = this.toggleAreaType.bind(this);
    }

    getValue() {
        return this.areasString;
    }

    toggleAreaType(type) {
        this.setState(() => ({typeSelected: type}));

        console.log({selectmode: this.state.selectModeActive});
        // if it's in selection mode
        if (this.state.selectModeActive) {
            // change the type of the selected area
            this.areas[this.state.areaSelectedIndex].type = type;
            this.redraw();
        }
    }

    componentDidMount() {
        super.componentDidMount();
        this.onReady(this.props.image, this.props.initialValue);
    }

    startSelectMode() {
        this.setState(() => ({selectModeActive: true}));
    }

    onSelect() {
        // set selection type to the same as the area
        if (this.state.typeSelected !== this.areas[this.state.areaSelectedIndex].type) {
            this.toggleAreaType(this.areas[this.state.areaSelectedIndex].type);
        }
    }

    removeSelected() {
        this.lastUndoEvents.push({
            type: "areas-removed",
            obj: this.areas.splice(this.state.areaSelectedIndex, 1),
        });
        this.redraw();
        this.exitSelectionMode();
    }

    exitSelectionMode() {
        this.setState(() => ({selectModeActive: false, areaSelectedIndex: null}));
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
    }

    save(save) {
        if (save) {
            this.areas2String();
        }

        this.x = this.y = 0;
        this.areas = [];
        this.areasString = "";
        this.current = {
            points: [],
            color: "",
        };
    }

    onReady(frame, initialValue /*, roi*/) {
        this.lastImage = frame;

        // var parsedRoi = this.stringToRoi(roi);
        // if (parsedRoi) {
        //     this.setCanvasSize({width: parsedRoi[2], height: parsedRoi[3]});
        // }

        // update this.areas
        this.string2areas(initialValue);

        this.updateCanvasPosition();

        this.redraw();
    }

    closeCurrentPoly() {
        // Saves the poly drawn into the areas collection
        /// TODO: Check if it's a complex polygon, e.g. has intersection between lines of the poly

        if (this.current.points.length === 0) return;

        if (!this.current.aproximated) {
            if (confirm(this.props.t("Aproximate polygon curve(s)? It can increases the program perfomance."))) {
                this.aproxPoly();
            }
        }

        var type = this.state.typeSelected;
        this.areas.push({type: type, points: this.current.points});
        this.current = {
            points: [],
            color: "",
            aproximated: false,
        };

        this.areas2String();
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
        // TODO: use this.repaintCanvas(callback)

        // Draws the canvas with the areas and current points
        var image = new Image();
        image.onload = () => {
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

    // Click or touch pressed
    pressed(e) {
        e.preventDefault();
        e = (e.touches || [])[0] || e; // get touch (mobile) or fall to e (mouse)
        const x = e.clientX - this.x;
        const y = e.clientY - this.y;

        if (!this.state.selectModeActive) {
            this.current.points.push({x, y});

            this.lastUndoEvents = [];

            this.redraw(false);
        } else {
            for (var ia in this.areas) {
                if (pointPolygonTest(this.areas[ia].points, {x, y}) > 0) {
                    this.setState(() => ({areaSelectedIndex: ia}));
                    // this.select.onSelect();
                }
            }
        }
    }

    render() {
        return (
            <ModalCanvas
                header={
                    <div className="message-header exclusivity-areas-header">
                        <p data-translation="Select the exclusivity areas of the camera">
                            Select the exclusivity areas of the camera
                        </p>

                        <div className="message-header-button">
                            {this.state.areaSelectedIndex === null && (
                                <button
                                    id="button-close-poly"
                                    className="button sizable"
                                    data-translation="Close polygon"
                                    onClick={() => this.closeCurrentPoly()}>
                                    Close polygon
                                </button>
                            )}

                            {this.state.areaSelectedIndex === null && (
                                <button
                                    id="button-aprox-poly"
                                    className="button sizable"
                                    data-translation="Aproximate polygon curve(s)"
                                    onClick={() => this.aproxPoly()}>
                                    Aproximate polygon curve(s)
                                </button>
                            )}

                            <div id="toggle-exclusivity-area-type" className="buttons has-addons selection">
                                <button
                                    className={"button " + (this.state.typeSelected === "allow" ? "is-warning" : "")}
                                    data-type="allow"
                                    onClick={() => this.toggleAreaType("allow")}>
                                    Allow points inside this poly
                                </button>
                                <button
                                    className={"button " + (this.state.typeSelected === "deny" ? "is-warning" : "")}
                                    data-type="deny"
                                    onClick={() => this.toggleAreaType("deny")}>
                                    Deny points inside this poly
                                </button>
                            </div>

                            {this.state.areaSelectedIndex === null && (
                                <div className="undo-redo sizable">
                                    <button id="button-undo" className="button" onClick={() => this.undo()}>
                                        <i className="fas fa-undo"></i>
                                        <p>Undo</p>
                                    </button>
                                    <button id="" className="button" onClick={() => this.redo()}>
                                        <p>Redo</p>
                                        <i className="fas fa-redo"></i>
                                    </button>
                                </div>
                            )}

                            {this.state.areaSelectedIndex === null && (
                                <button
                                    id="button-remove-all-exclareas"
                                    className="button sizable"
                                    data-translation="Remove all"
                                    onClick={() => this.removeAll()}>
                                    Remove all
                                </button>
                            )}

                            {this.state.areaSelectedIndex !== null && (
                                <button
                                    id="button-remove-selected-exclareas"
                                    className="button sizable selection"
                                    data-translation="Remove this area"
                                    onClick={() => this.removeSelected()}>
                                    Remove this area
                                </button>
                            )}

                            {this.state.areaSelectedIndex === null && (
                                <button
                                    id="button-start-selected-exclareas"
                                    className="button sizable"
                                    data-translation="Select area"
                                    onClick={() => this.startSelectMode()}>
                                    Select area
                                </button>
                            )}

                            {this.state.areaSelectedIndex !== null && (
                                <button
                                    id="button-exit-selected-exclareas"
                                    className="button sizable selection"
                                    data-translation="Exit from selection mode"
                                    onClick={() => this.exitSelectionMode()}>
                                    Exit from selection mode
                                </button>
                            )}
                        </div>
                    </div>
                }
                onAccept={this.props.onAccept}
                onCancel={this.props.onCancel}>
                <canvas ref={this.canvas} {...this.handlers} width="640" height="360" />
            </ModalCanvas>
        );
    }
}

CanvasExclusivityAreasHandler.propTypes = {
    t: PropTypes.any,
    image: PropTypes.string.isRequired,
    initialValue: PropTypes.string,
    callbackOnMounted: PropTypes.func,
    onCancel: PropTypes.func.isRequired,
    onAccept: PropTypes.func.isRequired,
};

export default CanvasExclusivityAreasHandler;
