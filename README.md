# Recognize
## **What is this?**
It started as a multi-platform program to alert about movement and people on cameras. And now it is the same but with several configurations to achieve that goal. These settings are made from a configuration file or from the graphical interface (wx app or web app).

## **Content**
The repository includes both the program that performs the recognition alerts and others, which is located in the folder "recognize". But it also has in the /src folder the files for the graphics configuration program.
You can compile "recognize" alone and run it with its parameters and it will work, that folder includes a CMakeLists.txt for that purpose. Although you can also compile the whole repository with the CMakeLists.txt from the root folder and use it from there.

This repository contains the 3 projects related to the recognize.
- **Recognize**: main program, shows the cameras, detects things and send alerts. Built with [`opencv`](https://opencv.org/) (capture video and then process and show those images), [`curl`](https://curl.se/) (send telegram notifications) and [`jsoncpp`](https://github.com/open-source-parsers/jsoncpp) (decode json from telegram bot messages).

	- **webRecognize**: Web-GUI program to make it easier to configure the main program, can start it, stop it and show notifications. Client side built with pure js and html with `bulma` (style), `moment` (dates) and `jquery` (js to html helper). Server side uses [`uWebSockets`](https://github.com/uNetworking/uWebSockets/) to serve the files in a http server and provide WebSockets connections, the server also uses fmt.

	- **wxRecognize**: First GUI made, then replaced with webRecognize so they have the same purpose. Built with [`wxWidgets`](wxwidgets.org).

# Libs/headers requerired	
- The ones described above.
- Check the README.md of each project folder.