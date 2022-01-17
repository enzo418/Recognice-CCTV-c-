import React from "react";

import i18n from "./i18n";
import CanvasRoiHandler from "./modules/canvas/canvas_handler_roi";
import CanvasAreasHandler from "./modules/canvas/canvas_handler_area";
import LiveView from "./modules/liveView/LiveView";
import CanvasExclusivityAreasHandler from "./modules/canvas/canvas_handler_exclAreas";
// import {w3cwebsocket as W3CWebSocket} from "websocket";
import { WrapperWebSocket } from "./modules/websocket_wrapper";

class App extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            liveViewScreen: true,
            uri_video: "Enter the uri",
            modalCanvas: {
                references: {
                    roi: React.createRef(),
                    ignoredAreas: React.createRef(),
                    exclusivityAreas: React.createRef(),
                },

                currentImage: "", // base64 encoded image to use

                initialValue: "",

                // saves the current handler
                // id is used to access references[activeHandlerId]
                activeHandlerId: "",

                // default handler, can change
                onAccept: () => this.hideCanvasModal(),

                // do not change cancel
                onCancel: () => this.hideCanvasModal(),
            }
        };


        this.socket = new WrapperWebSocket(`ws://${window.location.host}/recognize`);

        this.openModalCanvas = this.openModalCanvas.bind(this);
        this.onAcceptModalCanvas = this.onAcceptModalCanvas.bind(this);
        this.hideCanvasModal = this.hideCanvasModal.bind(this);
    }

    componentDidMount() {
        fetch("/api/configuration_files")
            .then((res) => res.json())
            .then(({configuration_files}) => {
                this.setState(
                    () => {
                        return {configurationFilesAvailables: configuration_files};
                    },
                    (error) => {
                        this.setState(() => ({error}));
                    }
                );
            });


        let lang = window.localStorage.getItem("lang") || "en";
        i18n.changeLanguage(lang);

        this.socket.on("notifications", (notifications) => {
            if (notifications) {
                if (this.state.configuration["playSoundOnNotification"]) {
                    this.playAudioAlert();
                }
            }
        });
    }

    hideCanvasModal() {
        this.setState((prev) => {
            prev.modalCanvas.activeHandlerId = "";
            return prev;
        });
    }

    /**
     * Opens a modal that contains a canvas
     * @param {string} canvasType roi|ignoredAreas|exclusivityAreas
     * @param {Function} onAccept callback when the user hits "Ok"
     * @param {string} image base64 encoded image
     */
    openModalCanvas(canvasType = "roi", onAccept, image, initialValue) {
        this.setState((prev) => {
            prev.modalCanvas.onAccept = onAccept;
            prev.modalCanvas.activeHandlerId = canvasType;
            prev.modalCanvas.currentImage = image;
            prev.modalCanvas.initialValue = initialValue;
            return prev;
        });
    }

    onAcceptModalCanvas() {
        // get value generated from the user input
        let value = this.state.modalCanvas.references[this.state.modalCanvas.activeHandlerId].current.getValue();

        // pass the accepted value to the canvas
        this.state.modalCanvas.onAccept(value);

        //
        this.hideCanvasModal();
    }

    onToggleLiveView() {
        this.setState(prevState => ({
            liveViewScreen: !prevState.liveViewScreen
        }));
    }

    handleInputChange(ev) {
        this.setState({ uri_video: ev.target.value });
    }

    onOpenConnection() {
        let camera = this.state.uri_video;
        let url;
        if (camera !== "observer") {
            url = `/api/requestCameraStream?uri=${encodeURIComponent(camera)}`;
        } else {
            url = "/api/requestObserverStream";
        }

        fetch(url)
            .then(response => response.json())
            .then(data => {
                if (data.status !== "error") {
                    this.setState(
                        () => {
                            return { feed_id: data["data"]["ws_feed_path"] };
                        },
                        (error) => {
                            this.setState(() => ({ error }));
                        }
                    );
                }
            });
    }

    render() {
        return (
            <div>
                {this.state.modalCanvas.activeHandlerId === "roi" && (
                    <CanvasRoiHandler
                        ref={this.state.modalCanvas.references.roi}
                        image={this.state.modalCanvas.currentImage}
                        initialValue={this.state.modalCanvas.initialValue}
                        onAccept={this.onAcceptModalCanvas}
                        onCancel={this.hideCanvasModal}></CanvasRoiHandler>
                )}

                <div>
                    <button onClick={this.onToggleLiveView.bind(this)}>Toggle Live View/Area selector</button>
                    <button onClick={this.onOpenConnection.bind(this)}>Open connection</button>
                    <input
                        type="text"
                        value={this.state.uri_video}
                        onChange={this.handleInputChange.bind(this)}
                        placeholder="Camera uri, or observer if use observer" />

                    {this.state.liveViewScreen && this.state.feed_id &&
                        <LiveView feed_id={this.state.feed_id} onLoad={() => { }} ></LiveView>
                    }

                    {!this.state.liveViewScreen && this.state.feed_id && (
                        <CanvasAreasHandler
                            ref={this.state.modalCanvas.references.ignoredAreas}
                            feed_id={this.state.feed_id}
                            onAccept={this.onAcceptModalCanvas}
                            onCancel={this.hideCanvasModal}
                        />)}
                </div>

                {this.state.modalCanvas.activeHandlerId === "exclusivityAreas" && (
                    <CanvasExclusivityAreasHandler
                        ref={this.state.modalCanvas.references.exclusivityAreas}
                        image={this.state.modalCanvas.currentImage}
                        initialValue={this.state.modalCanvas.initialValue}
                        onAccept={this.onAcceptModalCanvas}
                        onCancel={this.hideCanvasModal}
                    />
                )}
            </div>
        );
    }
}

App.propTypes = {
};

export default App;
