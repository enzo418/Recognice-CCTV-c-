# Recognize
An application software that detects objects and alerts the users for specific cameras. Cameras are added and configured from the [Web application](https://github.com/enzo418/WebRecognize).

## **Content**
The repository includes the detection library that performs the recognition and the backend server, located in the folders "recognize" and "webRecognize", respectively. 
Those projects can be compiled separately by using Cmake.

As said before, this repository contains 2 projects:

- **Recognize**: Detection library. 
	- It includes features such as showing cameras, detecting movement, blob tracking, and sending alerts to Telegram or a local server.
	- Built with:
		- [`opencv`](https://opencv.org/) capture video and then process and show those images
		- [`curl`](https://curl.se/) to send notifications
		- [`nlohmann/json`](https://github.com/nlohmann/json) to read/parse a configuration in JSON format

- **webRecognize**: 
	- A backend server that provides the frontend with a REST API.
	- Provides WebSocket endpoints  for subscribing to notifications and buffer events.
	- One of the marvelous things it's able to do is provide single-field configuration with automatic type validation.
	- Uses [`uWebSockets`](https://github.com/uNetworking/uWebSockets/) with a Controller pattern to provide the REST API.

The Web interface is stored in its own repository [here](https://github.com/enzo418/WebRecognize)
- **WebRecognize (GUI)**
	- Uses Typescript, React, and MUI (Material UI).
	- Uses a Service Layer to communicate with the backend.
	- Supports single-field configuration and automatic validation.
	- Real-time cameras view

There is another folder that has the wxWidgets implementation of the User Interface.
- **wxRecognize**: The first GUI created, which was later replaced by webRecognize. Built with [`wxWidgets`](wxwidgets.org).

## Note
Check the README.md of each project folder.