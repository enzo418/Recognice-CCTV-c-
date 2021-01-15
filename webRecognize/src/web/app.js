const getCameraAccordionTemplate = (i, camera) => `
<div class="card">
<header class="card-header">
	<p class="card-header-title">
		${camera["cameraname"]}
	</p>
	<a href="#collapsible-camera-${i}" data-action="collapse" class="card-header-icon is-hidden-fullscreen" aria-label="more options">
		<span class="icon">
			<i class="fas fa-angle-down" aria-hidden="true"></i>
		</span>
	</a>
</header>
<div id="collapsible-camera-${i}" class="is-collapsible" data-parent="accordion_first">
	<div class="card-content camera-config-content"> <!-- Start content card -->

<div class="card-content-item">
	<label for="cameraName">Camera name</label>
	<input class="input" name="cameraname" type="text" placeholder="camera name" value='${camera["cameraname"] || ""}'>
</div>

<div class="card-content-item">
	<label for="url">Camera url</label>
	<input class="input" name="url" type="text" placeholder="url" value='${camera["url"] || ""}'>
</div>

<div class="card-content-item">
	<label for="order" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Position of the camera in the preview. Is unique">Order of the camera in the preview</label>
	<input class="input" name="order" type="number" min="0" value='${camera["order"]}'>
</div>

<div class="card-content-item">
	<label for="rotation">Rotation of the camera, helps to detect objects correctly (degrees)</label>
	<input class="input" name="rotation" type="number" min="0" value='${camera["rotation"]}'>
</div>

<div class="card-content-item">
	<label for="framesToAnalyze" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Selects the frames to search a person on, from">Frames to search a person in the obtained frames, values need to be lower or equal to gif frames number respectively (before..after).</label>
	<input class="input" name="framestoanalyze" type="text" placeholder="5..60" value='${camera["framestoanalyze"] || ""}'>
</div>

<div class="card-content-item">
	<label for="type">Camera type: 
		<p>0 = Disabled, Camera is disabled, doesn't show or process frames.</p>
		<p>1 = Sentry, Only sends notifications.</p>
		<p>2 = Active, Same as Sentry but try to recognize a person in the frames selected on "framesToAnalyze".</p>
	</label>
	<input class="input" name="type" type="number" placeholder="url" min="0" max="2" value='${camera["type"]}'>
</div>

<div class="card-content-item">
	<label for="thresholdNoise">Used to remove noise (single scattered pixels). Between 30 and 50 is a general good value.</label>
	<input class="input" name="thresholdnoise" type="number" min="0" placeholder="45" value='${camera["thresholdnoise"]}'>
</div>

<div class="card-content-item">
	<label for="minimumThreshold">Minimum number of different pixels between the last 2 frames. Is used to leave a margin of "error". Is recommended to set it at a low number, like 10.You maybe will have to change it if you change the theshold noise or update Frequency</label>
	<input class="input" name="minimumthreshold" type="number" min="0" placeholder="10" value='${camera["minimumthreshold"]}'>
</div>

<div class="card-content-item">
	<label for="increaseTresholdFactor">Since the app is calculating the average change of pixels between the last two images you need to leave a margin to avoid sending notifications over small or insignificant changes. A general good value is between 1.04 (4%) and 1.30 (30%) of the average change.</label>
	<input class="input" name="increasetresholdfactor" type="number" min="0" placeholder="1" value='${camera["increasetresholdfactor"]}'>
</div>

<div class="card-content-item">
	<label for="updateThresholdFrequency">This tells the app how frequent (seconds) to update the average pixels change between the last two frames. On camera where there is fast changing objects is good to leave this value low, e.g. 5.</label>
	<input class="input" name="updatethresholdfrequency" type="number" min="0" placeholder="10" value='${camera["updatethresholdfrequency"]}'>
</div>

<div class="card-content-item">
	<label for="thresholdFindingsOnIgnoredArea">How many objects or changes on ignored areas are needed in order to not send a notification about the change?</label>
	<input class="input" name="thresholdfindingsonignoredarea" type="number" min="0" placeholder="2" value='${camera["thresholdfindingsonignoredarea"]}'>
</div>

<div class="card-content-item">
	<label for="minPercentageAreaNeededToIgnore">The change descriptor can have some 'error' due to filter and noise reduction, so is better to leave a margin in the area needed to match a ignored area for that. Recommended value: between 90 and 100.</label>
	<input class="input" name="minpercentageareaneededtoignore" type="number" min="0" placeholder="95" value='${camera["minpercentageareaneededtoignore"]}'>
</div>

<div class="card-content-item is-hidden">
	<label for="hitThreshold"></label>
	<input class="input" name="hitthreshold" type="number" value='${camera["hitthreshold"]}'>
</div>

<div class="card-content-item is-hidden">
	<label for="ignoredAreas"></label>
	<input class="input" name="ignoredareas" type="text" value='${camera["ignoredareas"] || ""}'>
</div>

<div class="card-content-item is-hidden">
	<label for="roi"></label>
	<input class="input" name="roi" type="text" value='${camera["roi"] || ""}'>
</div>

</div> <!-- End content card -->
	<footer class="card-footer">
		<div class="card-footer-item footer-camera-buttons">
			<button class="button">Select camera region of interest</button>
			<button class="button">Select camera ignored areas</button>
			<button class="button is-danger">Delete camera</button>
		</div>
		</div>
	</footer>
</div>
</div>`;

const getProgramAccordionTemplate = program => `
<div class="card">
	<header class="card-header">
		<p class="card-header-title">
			Program configuration
		</p>
		<a href="#collapsible-program-config" data-action="collapse" class="card-header-icon is-hidden-fullscreen" aria-label="more options">
			<span class="icon">
				<i class="fas fa-angle-down" aria-hidden="true"></i>
			</span>
		</a>
	</header>
	<div id="collapsible-program-config" class="is-collapsible" data-parent="accordion_first">
		<div class="card-content program-config-content">

<div class="card-content-item">
	<label for="msBetweenFrame" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Greater this value = Lower CPU usage.\n FPS = 1000 / ms, e.g. 25 ms = 40 FPS.\n Try different values and determine the efficiency vs effectiveness.">Milliseconds between frame of the camera</label>
	<input class="input" name="msbetweenframe" type="number" placeholder="30" min="0" value="${program['msbetweenframe']}">
</div>

<div class="card-content-item">
	<label for="msBetweenFrameAfterChange" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Same as <msBetweenFrame>. But only will be applied after detecting a change in the frames.">Milliseconds between frame of the camera after detecting a change</label>
	<input class="input" name="msbetweenframeafterchange" type="number" placeholder="30" min="0" value="${program['msbetweenframeafterchange']}">
</div>

<div class="card-content-item">
	<label for="outputResolution" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="width,height">Output-preview resolution</label>
	<input class="input" name="outputresolution" type="text" placeholder="640,360" value="${program['outputresolution'] || ""}">
</div>

<div class="card-content-item">
	<label for="ratioScaleOutput" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Scales the output">Ratio scale ouput-preview</label>
	<input class="input" name="ratioscaleoutput" type="number" placeholder="1" min="0" value="${program['ratioscaleoutput']}">
</div>

<div class="card-content-item">
	<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Draws the ignored area for each camera">
		<input type="checkbox" name="showignoredareas" ${program['showignoredareas'] == '1' && 'checked'}>
		Show ignored areas
	</label>
</div>

<div class="card-content-item">
	<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="show cameras">
		<input type="checkbox" name="showpreviewcameras" ${program['showpreviewcameras'] == '1' && 'checked'}>
		Show preview-output of the cameras
	</label>
</div>

<div class="card-content-item">
	<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Shows frames cropped rotated and with the change between them">
		<input type="checkbox" name="showprocessedframes" ${program['showprocessedframes'] == '1' && 'checked'}>
		Show processed cameras frames
	</label>
</div>

<div class="card-content-item">
	<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Draws the rectangle corresponding to the ROI of the camera">
		<input type="checkbox" name="showareacamerasees" ${program['showareacamerasees'] == '1' && 'checked'}>
		Draw the region of interest for each camera
	</label>
</div>

<div class="card-content-item">
<label class="checkbox">
	<input type="checkbox" name="uselocalnotifications" ${program['uselocalnotifications'] == '1' && 'checked'}>
	Use local notifications: if checked then the program will send notification (if send image when detect change or send text when detect change is checked)
	to the local client, when using webRecognize. Doesn't affect telegram notification, you can use one or another without problems.
</label>
</div>

<!-- Telegram -->
<div class="card-content-item">
	<label for="telegramBotApi">Telegram bot api key</label>
	<input class="input" name="telegrambotapi" type="text" value="${program['telegrambotapi'] || ""}">
</div>

<div class="card-content-item">
	<label for="telegramChatId">Telegram chat target id</label>
	<input class="input" name="telegramchatid" type="text" value="${program['telegramchatid'] || ""}">
</div>

<div class="card-content-item">
<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Disable this to not send notifications.">
	<input type="checkbox" name="usetelegrambot" ${program['usetelegrambot'] == '1' && 'checked'}>
	Use telegram bot
</label>
</div>

<div class="card-content-item">
<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Sends a image of all the cameras after detecting a change. Does not replace send image or send text after change.">
	<input type="checkbox" name="sendimageofallcameras" ${program['sendimageofallcameras'] == '1' && 'checked'}>
	Send image of all cameras after change
</label>
</div>

<div class="card-content-item">
	<label for="secondsBetweenImage" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Seconds to wait to send another notification with image.">Seconds between notification with image</label>
	<input class="input" name="secondsbetweenimage" type="number" placeholder="30" min="0" value="${program['secondsbetweenimage']}">
</div>

<div class="card-content-item">
	<label for="secondsBetweenMessage" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Seconds to wait to send another notification with only text.">Seconds between notification with text</label>
	<input class="input" name="secondsbetweenmessage" type="number" placeholder="30" min="0" value="${program['secondsbetweenmessage']}">
</div>

<div class="card-content-item">
	<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Send image message after detectig change.">
		<input type="checkbox" name="sendimagewhendetectchange" ${program['sendimagewhendetectchange'] == '1' && 'checked'}>
		Send a image when detect a change
	</label>
</div>

<div class="card-content-item">
	<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Send text message after detectig change.">
		<input type="checkbox" name="sendtextwhendetectchange" ${program['sendtextwhendetectchange'] == '1' && 'checked'}>
		Send a text alert when detect a change
	</label>
</div>

<div class="card-content-item">
	<label for="authUsersToSendActions" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="user_1,user_2,...,user_n.">Authorized users to send actions from telegram</label>
	<input class="input" name="authuserstosendactions" type="text" placeholder="user1,user2,usern" value="${program['authuserstosendactions'] || ""}">
</div>

<div class="card-content-item">
<label class="checkbox">
	<input type="checkbox" name="usegifinsteadofimage" ${program['usegifinsteadofimage'] == '1' && 'checked'}>
	Send a gif instead of a image
</label>
</div>

<div class="card-content-item">
	<label for="gifResizePercentage" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="Selects the Quality of the gif. Values go from 0 to 100. 50 is means that the gif will be resized at half the resolution..">Gif resize resolution percentage</label>
	<input class="input" name="gifresizepercentage" type="number" placeholder="50" min="0" max="100" value="${program['gifresizepercentage']}">
</div>

<div class="card-content-item">
	<label for="detectionMethod">Detection method to use: <p> 0: HOG Descriptor, uses built in opencv HOG Descriptor.</p> <p>1: YOLO V4 DNN, uses darknet neural net, more precise than HOG.</p></label>
	<input class="input" name="detectionmethod" type="number" placeholder="50" min="0" max="1" value="${program['detectionmethod']}">
</div>

<div class="card-content-item">
	<label for="gifFrames" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="How much frames are going to be on the GIF, \nbefore the detected change .. anfter the cange">Number of frames in the gif, before and after the change: nBefore..nAfter</label>
	<input class="input" name="gifframes" type="text" placeholder="5..60" value="${program['gifframes'] || ""}">
</div>

<div class="card-content-item">
	<label for="imagesFolder">Folder name where to save the images, relative to the executable path (if val=media then path will be <executable_path>/media)</label>
	<input class="input" name="imagesfolder" type="text" placeholder="media" value="${program['imagesfolder']}">
</div>

</div>
</div>
</div>`;

const getNotificationTemplate = (type, text) => `
<div class="box">
${(type == "image" && 
	`<figure class="image">
		<img src="${text}" alt="Image">
	</figure>`
) || ""}
${(type == "text" && `<h3 class="subtitle is-3">${text}</h3>`) || ""}
<h6 class="subtitle is-6 hour">${moment().format('MMMM Do YYYY, h:mm:ss a')}</h6>
</div>`;

var FILE_PATH = ""; // file that the user requested at the start
var RECOGNIZE_RUNNING = false;
var IS_NOTIFICATION_PAGE = false; // showing notitifcation page
var ws;

$(function() {
	ws = new WebSocket('ws://' + document.location.host + '/file');

	ws.onopen = function() {
		console.log('onopen');
		$('#pageloader').removeClass("is-active");
		$('#modal-file').addClass("is-active");
		
		var dropdown = document.querySelector('#dropdown-file');
		var dropdown_content = dropdown.querySelector('.dropdown-content');
		dropdown.addEventListener('click', function(event) {
			event.stopPropagation();
			dropdown.classList.toggle('is-active');
		});
	};

	ws.onclose = function() {
		$('#message').text('Lost connection.');
		console.log('onclose');
	};

	ws.onmessage = function(message) {
		var data = JSON.parse(message.data);
		console.log(data);

		if (data.hasOwnProperty("configuration_files")) {
			var dropdown_content = document.querySelector('#dropdown-file div.dropdown-content');
			data["configuration_files"].forEach((file) => {
				var file_elem = document.createElement("a");
				
				file_elem.classList.add("dropdown-item");
				
				file_elem.innerText = file;

				file_elem.onclick = (e) => {
					var children = [...dropdown_content.children]; // get array of childrens
					$('#dropdown-current-file-element').text(file); // set text of dropdown to file
					children.forEach(node => { // remove last active element class
						if (node.classList.contains("is-active"))
							node.classList.toggle("is-active");
					})
					file_elem.classList.add("is-active"); // set active class to this file elem
				};

				dropdown_content.appendChild(file_elem); // add file elem to dropdown
			});
		} else if (data.hasOwnProperty("configuration_file")) {
			console.log(data);
			setTimeout(() => $('#modal-file').removeClass('is-active'), 50);

			//  Add accordion program and cameras
			var headers = getHeadersFromStringConfig(data["configuration_file"]);

			$('#accordion_first').append(getProgramAccordionTemplate(headers["program"]));

			headers["cameras"].forEach((val, i) => {
				$('#accordion_first').append(getCameraAccordionTemplate(i, val));
			});

			// Accordion
			const bulmaCollapsibleInstances = bulmaCollapsible.attach('.is-collapsible');

			// Loop into instances
			bulmaCollapsibleInstances.forEach(bulmaCollapsibleInstance => {
				// Check if current state is collapsed or not
				if (!bulmaCollapsibleInstance.collapsed()) {
					
				}
			});
		} else if (data.hasOwnProperty("recognize_state_changed")) {
			RECOGNIZE_RUNNING = data["recognize_state_changed"];
			changeRecognizeStatusElements(RECOGNIZE_RUNNING);
		} else if (data.hasOwnProperty("new_notification")) {
			var ob = data["new_notification"];
			console.log("Notification: ", ob);
			if (ob["type"] != "sound") {
				var not_content = document.getElementById('notifications-content');
				if (not_content.children.length > 0)
					$(getNotificationTemplate(ob["type"], ob['content'])).insertBefore(not_content.children[0]);
				else 
					$('#notifications-content').append(getNotificationTemplate(ob["type"], ob['content']));
				
				Push.create("Alert!", {
					body: "How's it hangin'?",
					icon: '/favicon.svg',
					timeout: 2000,
					onClick: function () {
						window.focus();
						this.close();
					}
				});
			}
			
			// if (playsound)
			var audio = new Audio('https://github.com/zhukov/webogram/blob/master/app/img/sound_a.mp3?raw=true');
			audio.volume = 0.5;
			audio.play();
		}
		
	};

	ws.onerror = function(error) {
		console.log('onerror ' + error);
		console.log(error);
	};

	$('#button-select-config-file').click(function() {
		$(this).addClass("is-loading");
		var selected = document.querySelector('#dropdown-file div.dropdown-content .is-active').innerText;
		FILE_PATH = selected;
		ws.send('need_config_file ' + selected);
		console.log("send: ", 'need_config_file ' + selected);
	});

	$('#button-toggle-recognize').click(function() {
		ws.send("change_recognize_state " + !RECOGNIZE_RUNNING);
	});
});

set = function(value) {
	$('#count').val(value)
}

function changeRecognizeStatusElements(running) {
	if (running) {
		$('#button-toggle-recognize').removeClass("is-sucess").addClass("is-danger").text("Stop recognize");
		$('#button-state-recognize').text("Recognize is running");
	} else {
		$('#button-toggle-recognize').removeClass("is-danger").addClass("is-sucess").text("Start recognize");
		$('#button-state-recognize').text("Recognize is not running");
	}
}

function getHeadersFromStringConfig(str) {
	var re = /(PROGRAM|CAMERA)/g;
	var headers_match = []
	while ((match = re.exec(str)) != null) {
		var start = match.index - 2;
		var end = match.index + match[0].length + 2;
		var name = match[0]
		headers_match.push({start, end, name});
	}

	var headers = {"program": {}, "cameras": []};
	for(var i = 0; i < headers_match.length; i++) {
		var nxt = headers_match[i+1] || []
		
		var cam_str = str.slice(headers_match[i]["end"], nxt["start"] || str.length);
		
		var obj = {};
		var lines = cam_str.split('\n');
		for(var j = 0;j < lines.length; j++){
			if (lines[j].length > 0) {
				var eq = lines[j].indexOf("=");
				var id = lines[j].slice(0, eq).toLowerCase();
				var val = lines[j].slice(eq+1, lines[j].length);
				obj[id] = val;
			}
		}
		
		if (headers_match[i]["name"] == "CAMERA")
			headers["cameras"].push(obj);
		else 
			headers["program"] = obj;
	}
	
	return headers;
}

function saveIntoFile () {
	var programConfig = "[PROGRAM]\n";
	[...document.querySelectorAll('.program-config-content input')]
		.forEach(input => { 
			if (input.value.length > 0)
				programConfig +=`${input.name}=${input.type === "checkbox" ? (input.checked ? "1" : "0") : input.value}\n`
		});
	
	var camerasConfig = "";
	[...document.querySelectorAll('.camera-config-content')].forEach(cameraContent => {
		var config = "\n[CAMERA]\n";
		
		[...cameraContent.querySelectorAll('input')]
			.forEach(input => {
				if (input.value.length > 0)
					config +=`${input.name}=${input.type === "checkbox" ? (input.checked ? "1" : "0") : input.value}\n`
			});

		camerasConfig += config;
	});

	var config = programConfig + camerasConfig;


	ws.send('save_into_config_file ' + config);
}

function togglePage() {
	IS_NOTIFICATION_PAGE = !IS_NOTIFICATION_PAGE;

	$('#notifications-page').toggleClass("is-hidden");
	$('#configuration-page').toggleClass("is-hidden");

	if (IS_NOTIFICATION_PAGE) {
		$('#button-current-page').text("Show configurations page");
	} else {
		$('#button-current-page').text("Show notifications page");
	}
}

document.addEventListener('DOMContentLoaded', () => {

	// Get all "navbar-burger" elements
	const $navbarBurgers = Array.prototype.slice.call(document.querySelectorAll('.navbar-burger'), 0);

	// Check if there are any navbar burgers
	if ($navbarBurgers.length > 0) {

		// Add a click event on each of them
		$navbarBurgers.forEach( el => {
		el.addEventListener('click', () => {

			// Get the target from the "data-target" attribute
			const target = el.dataset.target;
			const $target = document.getElementById(target);

			// Toggle the "is-active" class on both the "navbar-burger" and the "navbar-menu"
			el.classList.toggle('is-active');
			$target.classList.toggle('is-active');

		});
		});
	}
});  