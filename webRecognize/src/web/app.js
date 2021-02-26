const getCameraContainerTemplate = (i, camera) => `
<div class="card is-hidden" id="camera-${i}">
	<header class="card-header">
		<p class="card-header-title">
			${camera["cameraname"]}
		</p>
	</header>
	
	<div class="card-content camera-config-content"> <!-- Start content card -->
	</div> <!-- End content card -->

	<footer class="card-footer">
		<div class="card-footer-item footer-camera-buttons">
			<button class="button button-select-camera-roi" 
					onclick="selectCameraROI(event, ${i})"
					data-translation="Select camera region of interest">Select camera region of interest</button>

			<button class="button button-select-camera-ignored-areas" 
					onclick="selectCameraIgnoredAreas(event, ${i})"
					data-translation="Select camera ignored areas">Select camera ignored areas</button>

			<button class="button is-danger" 
					onclick="deleteCamera(event, ${i})"
					data-translation="Delete camera">Delete camera</button>
		</div>
		</div>
	</footer>
</div>`;

const getProgramContainerTemplate = () => `
<div class="card" id="program">
	<header class="card-header">
		<p class="card-header-title" data-translation="Program configuration">Program configuration</p>
	</header>
	<div class="card-content program-config-content">

	</div>
</div>`;

const getTextInputItemTemplate = ($name, $placeholder, $value, $label, $tooltip, $hidden) => `
<div class="card-content-item ${$hidden && 'is-hidden'}" id="${$name.toLowerCase()}"  ${$tooltip.length > 0 && "class=has-tooltip-multiline has-tooltip-text-centered has-tooltip-right data-tooltip=\"" + $tooltip}">
	<label for="${$name}">${$label}</label>
	<input class="input" name="${$name}" type="text" placeholder="${$placeholder}" value="${$value}">
</div>`;

const getNumberInputItemTemplate = ($name, $placeholder, $value, $label, $tooltip, $max, $min, $hidden) => `
<div class="card-content-item ${$hidden && 'is-hidden'}" id="${$name.toLowerCase()}" ${$tooltip.length > 0 && "class=has-tooltip-multiline has-tooltip-text-centered has-tooltip-right data-tooltip=\"" + $tooltip}">
	<label for="${$name}">${$label}</label>
	<input class="input" name="${$name}" type="number" placeholder="${$placeholder}" ${$min != null && 'min="' + $min + '"'} ${$max != null && 'max="' + $max + '"'} ${$value && "value='" + $value + "'"}>
</div>`;

const getCheckBoxItemTemplate = ($name, $checked, $label, $tooltip, $hidden) => `
<div class="card-content-item ${$hidden && 'is-hidden'}" id="${$name.toLowerCase()}" ${$tooltip.length > 0 && "class=has-tooltip-multiline has-tooltip-text-centered has-tooltip-right data-tooltip=\"" + $tooltip}">
	<label class="checkbox">
		<input type="checkbox" name="${$name}" ${$checked && 'checked'}>
		${$label}
	</label>
</div>`;

const getNotificationGroupTemplate = (group_id) => `
<div id="${group_id}" class="box"></div>
`

const getNotificationTemplate = (type, text) => `
<div class="box ${(type == "text" && "text-notification") || ""}">
	${(type == "image" &&
		`<figure class="image">
		<img src="" data-src="${text}" alt="Image">
	</figure>`
	) || ""}
	
	${(type == "text" && `<h3 class="subtitle is-3">${text}</h3>`) || ""}
	
	${(type == "video" &&
	`<video width="640" height="360" preload="metadata" controls data-src="${text}"></video>`
	) || ""}

<h6 class="subtitle is-6 hour" data-date="${moment().format('MMMM Do YYYY, h:mm:ss a')}">now</h6>
</div>`;

const getTabTemplate = (i, camName) => `
<li data-config="camera-${i}" id="tab-camera-${i}">
	<a><span>${camName}</span></a>
</li>`

const getAlertTemplate = ($message, $stateClass) => `
<div class="notification ${$stateClass}">
	<button class="delete" onclick="deleteAlert(event)"></button>
	<span class="notification-text">${$message}</span>
</div>`;

const getGroupTemplate = ($name, $id) => `
<fieldset class="configuration-group" id=${$id}>
  <legend>${$name}</legend>
 </fieldset>`;

var CURRENT_LANG = localStorage.getItem("lang") || "es";
var FILE_PATH = ""; // file that the user requested at the start
var ROOT_CONFIGURATIONS_DIRECTORY = "./configurations/";
var RECOGNIZE_RUNNING = false;
var IS_NOTIFICATION_PAGE = false; // showing notitifcation page
var CONFIGURATION_HEADERS = {};
var ws;
var lastConfigurationActive = "program"; // id of the element
var cameras = []; // actual cameras as an dictionary
var unfinishedRequests = {}; 	// dict of unfinished request where key is the request and key a function to
// execute when we receive it.

var SEND_PUSH_NOTIFICATIONS = window.localStorage.getItem("send_push") === "true";
var PLAY_SOUND_NOTIFICATION = window.localStorage.getItem("play_sound_notification") === "true";

var cnvRoi = {
	canvas: null,
	ctx: null,
	lastImage: "",
	clickPressed: false,
	p1: { x: 0, y: 0 }, // lt or rb
	p2: { x: 0, y: 0 }, // rb or lt
	x: 0, // canvas position in x
	y: 0, // canvas position in y (with scrollbar)
	roi: ""
}

var cnvAreas = {
	canvas: null,
	ctx: null,
	lastImage: "",
	clickPressed: false,
	x: 0,
	y: 0,
	areas: [], // area: {lt: {x, y}, width, height}
	current: {
		p1: { x: 0, y: 0 },
		p2: { x: 0, y: 0 },
		color: ""
	},
	areasString: "",
	lastClick: null,
	colors: ["Aqua", "Red", "Blue", "Chartreuse", "Crimson", "Cyan", "DeepPink", "Gold", "LawnGreen", "PaleTurquoise"],
	removeAll: function () {
		this.areas = [];
		this.areasString = "";
		var camindex = $('#modal-igarea').data("index");
		document.querySelector('#camera-' + camindex)
			.querySelector('input[name="ignoredareas"]')
			.value = "";

		var image = new Image();
		image.onload = () => this.ctx.drawImage(image, 0, 0);

		image.src = "data:image/jpg;base64," + this.lastImage;
	}
}

var notificationPaginator = { index: 0, elements: [] }; // DOM notification elements

var configurationsElements = { elements: {}, translations: {} };

var appTitle = "Web Recognize";
var currentNumberNotificationsWindowTitle = 0;

var PendingElementsToToggleState = []; // array of {state: string, elements: [{id: string, on_checked: string, on_unchecked: string}]}

function _($key_string) {
	var lw = $key_string.toLowerCase();
	if (lw in configurationsElements.translations[CURRENT_LANG])
		return configurationsElements.translations[CURRENT_LANG][lw];
	else
		return $key_string;
}

function changeLanguage($clicked_el) {
	$lang_code = $clicked_el.dataset.lang;
	localStorage.setItem("lang", $lang_code);
	if (confirm(_("Is it necessary to restart the page to change the language, restart now?"))) {
		location.reload();
	} else {
		var drop_cont = document.getElementById('dropdown-language').querySelector('.dropdown-content');
		[...drop_cont.children].forEach(el => el.classList.remove('is-active'));
		$clicked_el.classList.add('is-active');
	}
}

function translateDOMElements() {
	document.querySelectorAll('[data-translation]').forEach(el => {
		el.innerText = _(el.dataset.translation.trim());
	});
}

$(function () {
	ws = new WebSocket('ws://' + document.location.host + '/file');

	ws.onopen = function () {
		console.log('onopen');
		$('#pageloader').removeClass("is-active");
		$('#modal-file').addClass("is-active");
	};

	ws.onclose = function () {
		$('#message').text(_('Lost connection'));
		console.log('onclose');
	};

	ws.onmessage = function (message) {
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
		}

		if (data.hasOwnProperty("configuration_file")) {
			setTimeout(() => $('#modal-file').removeClass('is-active'), 50);
			
			// if it was a copy
			if ("need_copy_file" in unfinishedRequests) {
				$('#modal-file-copy').toggleClass('is-active');
				delete unfinishedRequests["need_copy_file"];
			}

			//  Add accordion program and cameras
			CONFIGURATION_HEADERS = getHeadersFromStringConfig(data["configuration_file"]);

			cameras = CONFIGURATION_HEADERS.cameras;

			// -- ADD PROGRAM CONFIGURATION TO THE CONFIGURATIONS
			// create program configurations html-element	
			var programEl = $(getProgramContainerTemplate());
			var programContent = $(programEl).children('.program-config-content');

			addGroups(configurationsElements.elements.program.groups, CONFIGURATION_HEADERS.program, programContent, configurationsElements.translations[CURRENT_LANG]);

			$('#configurations').append(programEl);
	
			// -- ADD CAMERAS CONFIGURATIONS and tabs
			CONFIGURATION_HEADERS["cameras"].forEach((val, i) => addCameraConfigurationElementsTab(val, i));

			// Toggle elements that couldn't be disabled/enabled when needed
			PendingElementsToToggleState.forEach(toggle => {
				toggleStateElements(toggle.state, toggle.elements);
			});

			// translate new elements
			translateDOMElements();
		}

		if (data.hasOwnProperty("recognize_state_changed")) {

			// get Translations
			getElementsTranslations().then(res => {
				const [elements, translations] = res;
				configurationsElements.elements = elements;
				configurationsElements.translations = translations;

				// Translation
				translateDOMElements();

				$('#button-toggle-recognize').removeClass('is-loading');
				RECOGNIZE_RUNNING = data["recognize_state_changed"];
				changeRecognizeStatusElements(RECOGNIZE_RUNNING);
			});
		}

		if (data.hasOwnProperty("new_notification")) {
			var ob = data["new_notification"];
			console.log("Notification: ", ob);
			if (ob["type"] != "sound") {
				createNewNotification(ob["type"], ob["content"], true, ob["video"], ob["group"]);
				createAlert("ok", "New notification", 1500);
			}

			// only change if user is watching the last one
			if (notificationPaginator.index === notificationPaginator.elements.length - 1) {
				changeCurrentElementNotification(notificationPaginator.elements.length - 1);
			}

			if (PLAY_SOUND_NOTIFICATION) {
				var audio = new Audio('https://github.com/zhukov/webogram/blob/master/app/img/sound_a.mp3?raw=true');
				audio.volume = 0.5;
				audio.play();
			}

			if (document.visibilityState == "hidden") {
				document.title = `(${++currentNumberNotificationsWindowTitle}) ${appTitle}`;
			}
		}

		if (data.hasOwnProperty('request_reply')) {
			var ob = data["request_reply"];
			var $message = _(ob["message"]);
			if (ob["extra"].length > 0 ) {
				$message += `<br> ${_("Details")}: ${ob["extra"]}`;
			}

			createAlert(ob["status"], $message);
			if (ob["trigger"].length > 0) {
				// execute pending unifished request
				Object.entries(unfinishedRequests).forEach($request => {
					if ($request[0] == ob["trigger"]) {
						unfinishedRequests[$request[0]]();
						delete unfinishedRequests[$request[0]];
					}
				});
			}
		}

		if (data.hasOwnProperty('frame_camera')) {
			var ob = data['frame_camera'];

			var index = ob["camera"];
			var frame = ob["frame"];

			if (frameRequestOrigin === "roi") {
				var modal = document.querySelector('#modal-roi');

				modal.classList.add('is-active');

				modal.dataset.index = index;

				var image = new Image();
				image.onload = function () {
					cnvRoi.ctx.drawImage(image, 0, 0);

					cnvRoi.ctx.strokeStyle = "Red";
					cnvRoi.ctx.lineWidth = 5;

					var camera = document.querySelector('#camera-' + index);
					var roiInput = camera.querySelector('input[name="roi"]');
					cnvRoi.roi = roiInput.value;
					if (roiInput.value.length > 0) {
						var roi = stringToRoi(roiInput.value);
						if (roi.length > 0)
							cnvRoi.ctx.strokeRect(roi[0], roi[1], roi[2], roi[3]);
					}

					onResize();
				};
				image.src = "data:image/jpg;base64," + frame;

				cnvRoi.lastImage = frame;
			} else {
				cnvAreas.areas = [];

				var modal = document.querySelector('#modal-igarea');

				modal.classList.add('is-active');

				modal.dataset.index = index;

				var image = new Image();
				image.onload = function () {
					cnvAreas.ctx.drawImage(image, 0, 0);

					cnvAreas.ctx.strokeStyle = "Red";
					cnvAreas.ctx.lineWidth = 5;

					var camera = document.querySelector('#camera-' + index);
					var igAreas = camera.querySelector('input[name="ignoredareas"]');
					cnvAreas.areasString = igAreas.value;
					if (cnvAreas.areasString.length > 0) {
						var numbers = cnvAreas.areasString.match(/\d+/g).map(i => parseInt(i));
						if (numbers.length % 4 === 0) {
							for (var base = 0; base < numbers.length; base += 4) {
								const color = cnvAreas.colors[getRandomArbitrary(0, cnvAreas.colors.length)];
								const lt = { x: numbers[base + 0], y: numbers[base + 1] },
									width = numbers[base + 2],
									heigth = numbers[base + 3];

								cnvAreas.areas.push({ lt, width, heigth, color });

								cnvAreas.ctx.strokeStyle = color;
								cnvAreas.ctx.strokeRect(lt.x, lt.y, width, heigth);
							}
						}
					}

					console.log("areas loaded: ", cnvAreas.areas.length);
					onResize();
				};
				image.src = "data:image/jpg;base64," + frame;

				cnvAreas.lastImage = frame;
			}
		}

		if (data.hasOwnProperty("new_camera_config")) {
			var ob = data["new_camera_config"];
			console.log(ob);
			var headers = getHeadersFromStringConfig(ob["configuration"]);
			var i_start = cameras.length;
			headers.cameras.forEach((cam, i) => addCameraConfigurationElementsTab(cam, i_start + i));
		}

		if (data.hasOwnProperty("last_notifications")) {
			var ob = data["last_notifications"]["notifications"];
			ob.forEach(not => {
				if (not["type"] !== "sound")
					createNewNotification(not["type"], not["content"], false, not["group"])
			});

			changeCurrentElementNotification(notificationPaginator.elements.length - 1);
		}

		if (data.hasOwnProperty("root_configurations_directory")) {
			ROOT_CONFIGURATIONS_DIRECTORY = data["root_configurations_directory"];
			$('.root-dir-configurations').text(ROOT_CONFIGURATIONS_DIRECTORY);
		}
	};

	ws.onerror = function (error) {
		console.log('onerror ' + error);
		console.log(error);
	};

	$('#button-select-config-file').click(function () {
		$(this).addClass("is-loading");
		var selected = document.querySelector('#dropdown-file div.dropdown-content .is-active').innerText.trim();
		FILE_PATH = selected;

		// if new open modal to create new file
		if (selected == _("new")) {
			$('#modal-file-name').toggleClass('is-active');
			$('#modal-file').toggleClass('is-active');
			$(this).toggleClass("is-loading");	
		} else {
			// else request the file
			sendObj('need_config_file', { file: selected, is_new: false});
			
			unfinishedRequests["need_config_file"] = () => {				
				setTimeout(() => {
					$(this).removeClass("is-loading");
				}, 200);
			}
		}
	});

	$('#button-toggle-recognize').click(function () {
		if (FILE_PATH.length === 0 && !RECOGNIZE_RUNNING) {
			createAlert("error", _("You can not start the recognize without selecting a configuration file"), 5000);
		} else {
			$(this).toggleClass('is-loading');
			sendObj("change_recognize_state", { state: !RECOGNIZE_RUNNING });
			
			unfinishedRequests["change_recognize_state"] = () => {				
				setTimeout(() => {
					$(this).removeClass("is-loading");
				}, 200);
			}
		}
	});

	$('#button-just-notifications').click(function () {
		$('#configuration-page').remove();
		$('#button-current-page').remove();
		togglePage();
		$('#modal-file').toggleClass('is-active');
		Notification.requestPermission();
	});

	$('#button-modal-make-copy-file').click(function () {
		var selected = document.querySelector('#dropdown-file div.dropdown-content .is-active').innerText;
		if (selected !== _("new")) {
			FILE_PATH = selected;
			$('#modal-file-copy').toggleClass('is-active');
			$('#modal-file').toggleClass('is-active');
		} else {
			createAlert("error", _("Cannot create a copy of a non existing file"), 3000);
		}
	});

	$('#button-cancel-copy-file').click(function () {
		$('#modal-file-copy').toggleClass('is-active');
		$('#modal-file').toggleClass('is-active');
	});

	$('#button-make-copy-file').click(function (event) {
		event.preventDefault();
		$(this).addClass("is-loading");
		var selectedFile = FILE_PATH;
		FILE_PATH = ROOT_CONFIGURATIONS_DIRECTORY + ($('#file-copy-name').val()).replace(/(\.\w+)+/, '') + ".ini";
		sendObj('need_copy_file', { file: selectedFile, copy_path: FILE_PATH });
				
		unfinishedRequests["need_copy_file"] = () => {				
			setTimeout(() => {
				$(this).removeClass("is-loading");

				$('#modal-file-copy').toggleClass('is-active');
				$('#modal-file').toggleClass('is-active');
			}, 200);
		}
	});

	$('#button-cancel-file-name').click(function () {
		$('#modal-file-name').toggleClass('is-active');
		$('#modal-file').toggleClass('is-active');
	});

	$('#button-selected-new-file-name').click(function () {
		$(this).addClass("is-loading");
		FILE_PATH = ROOT_CONFIGURATIONS_DIRECTORY + ($('#new-file-name').val()).replace(/(\.\w+)+/, '') + ".ini";
		sendObj('need_config_file', { file: FILE_PATH, is_new: true });
		$('#modal-file-name').toggleClass('is-active');
	});
});

function stringToRoi(roi) {
	var numbers = roi.match(/\d+/g).map(i => parseInt(i));
	if (numbers.length === 4)
		return [numbers[0], numbers[1], numbers[2], numbers[3]];
	else
		return false;
}

function togglePush() {
	SEND_PUSH_NOTIFICATIONS = !SEND_PUSH_NOTIFICATIONS;
	window.localStorage.setItem("send_push", SEND_PUSH_NOTIFICATIONS);
}

function toggleNotificationSound() {
	PLAY_SOUND_NOTIFICATION = !PLAY_SOUND_NOTIFICATION;
	window.localStorage.setItem("play_sound_notification", PLAY_SOUND_NOTIFICATION);
}

function createAlert($state, $message, $duration = 8000) {
	var stateClass = $state === "ok" ? 'is-success' : 'is-danger';
	var alert = $(getAlertTemplate($message, stateClass));
	$('#alerts').append(alert);

	setTimeout(() => alert.remove(), $duration);
}

function deleteAlert($ev) {
	($ev.target.parentNode).remove();
}

function createNewNotification($type, $content, $sendPush, $groupID) {
	var $not = $(getNotificationTemplate($type, $content));

	$('.navigator-notification').removeClass('is-hidden');
	
	if ($groupID > 0) {
		let found = notificationPaginator.elements.find(el => el.id && el.id == $groupID);
		if (found) {
			found.root.append($not);
		} else {
			var root = $(getNotificationGroupTemplate($groupID));
			
			this.notificationPaginator.elements.push(
				{
					"id": $groupID, 
					"root": root
				}
			);

			root.append($not);
		}
	} else {
		this.notificationPaginator.elements.push($not[0]);
	}

	if (SEND_PUSH_NOTIFICATIONS && $sendPush) {
		Push.create(_("New notification"), {
			body: "...",
			icon: '/assets/favicon.svg',
			link: '/#',
			timeout: 4000,
			onClick: function () {
				window.focus();
				this.close();
			},
			vibrate: [200, 100, 200, 100, 200, 100, 200]
		});
	}

	updateNotificationsNumberPaginator();
}

function sendObj(key, body) {
	body["key"] = key;
	ws.send(JSON.stringify(body));
	console.log("Sended: ", body);
}

function onTabClick($e) {
	var el = $e.currentTarget;

	document.getElementById(lastConfigurationActive).classList.add('is-hidden');

	// set listeners tabs
	[...document.querySelector('.tabs ul').querySelectorAll('li')]
		.forEach(el => el.classList.remove('is-active'))

	el.classList.add('is-active');

	document.getElementById(el.dataset.config).classList.remove('is-hidden');

	lastConfigurationActive = el.dataset.config;
}

function addCameraConfigurationElementsTab(val, i) {
	var tabs = document.querySelector('.tabs ul');
	// add tab
	var $tab = $(getTabTemplate(i, val["cameraname"]));
	$(tabs).append($tab);
	$tab.click(onTabClick);

	// get camera root container
	var camEl = $(getCameraContainerTemplate(i, val));
	var camConten = $(camEl).children('.camera-config-content');

	addGroups(configurationsElements.elements.camera.groups, val, camConten, configurationsElements.translations[CURRENT_LANG])

	// add to configurations container
	$('#configurations').append(camEl);
}

function changeRecognizeStatusElements(running) {
	if (running) {
		$('#button-toggle-recognize').removeClass("is-sucess").addClass("is-danger").text(_("Stop recognizer"));
		$('#button-state-recognize').text(_("Recognizer is running"));
	} else {
		$('#button-toggle-recognize').removeClass("is-danger").addClass("is-sucess").text(_("Start recognizer"));
		$('#button-state-recognize').text(_("Recognizer is not running"));
	}
}

function getHeadersFromStringConfig(str) {
	var re = /(PROGRAM|CAMERA)/g;
	var headers_match = []
	while ((match = re.exec(str)) != null) {
		var start = match.index - 2;
		var end = match.index + match[0].length + 2;
		var name = match[0]
		headers_match.push({ start, end, name });
	}

	var headers = { "program": {}, "cameras": [] };
	for (var i = 0; i < headers_match.length; i++) {
		var nxt = headers_match[i + 1] || []

		var cam_str = str.slice(headers_match[i]["end"], nxt["start"] || str.length);

		var obj = {};
		var lines = cam_str.split('\n');
		for (var j = 0; j < lines.length; j++) {
			if (lines[j].length > 0) {
				var eq = lines[j].indexOf("=");
				var id = lines[j].slice(0, eq).toLowerCase();
				var val = lines[j].slice(eq + 1, lines[j].length);
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

function saveIntoFile() {
	var programConfig = "[PROGRAM]\n";
	[...document.querySelectorAll('.program-config-content input')]
		.forEach(input => {
			if (input.value.length > 0)
				programConfig += `${input.name}=${input.type === "checkbox" ? (input.checked ? "1" : "0") : input.value}\n`
		});

	var camerasConfig = "";
	[...document.querySelectorAll('.camera-config-content')].forEach(cameraContent => {
		var config = "\n[CAMERA]\n";

		[...cameraContent.querySelectorAll('input')]
			.forEach(input => {
				if (input.value.length > 0)
					config += `${input.name}=${input.type === "checkbox" ? (input.checked ? "1" : "0") : input.value}\n`
			});

		camerasConfig += config;
	});

	var configurations = programConfig + camerasConfig;

	sendObj('save_into_config_file', { configurations });
}

function togglePage() {
	IS_NOTIFICATION_PAGE = !IS_NOTIFICATION_PAGE;

	$('#notifications-page').toggleClass("is-hidden");
	$('#configuration-page').toggleClass("is-hidden");

	if (IS_NOTIFICATION_PAGE) {
		$('#button-current-page').text(_("Show configurations page"));
	} else {
		$('#button-current-page').text(_("Show notifications page"));
	}
}

function getElementsTranslations() {
	const objectKeysToLowerCase = obj => Object.fromEntries(
		Object.entries(obj).map(([k, v]) => [k.toLowerCase(), v])
	)

	return new Promise(resolve => {
		fetch('/elements.json')
			.then(elements => elements.json())
			.then(elements => {
				// lower case ids of the elements
				groupsToLowerCase(elements.camera.groups);
				groupsToLowerCase(elements.program.groups);

				// get the translations
				fetch('/translations.json')
					.then(translations => translations.json())
					.then(translations => {
						Object.entries(translations).forEach(
							el => translations[el[0]] = objectKeysToLowerCase(translations[el[0]]));

						resolve([elements, translations]);
					})
			})
	});
}

function groupsToLowerCase(groups) {
	[...groups].forEach(group => {
		group.elements.forEach(el => { el.target = el.target.toLowerCase() })

		if (group.groups) {
			groupsToLowerCase(group.groups);
		}
	});
}

function addTemplateElements(jqElemRoot, values, elements, translations) {
	Object.entries(elements).forEach(el => {
		const $name = el[0].target;
		appendTemplateElement(jqElemRoot, values[$name], el, translations[$name].label, translations[$name].description);
	});
}

function appendTemplateElement($jqElemRoot, $value, $element, $label, $description) {
	const $name = $element.target;
	const $type = $element.type;
	const $hidden = $element.hidden;
	const $placeholder = $element.placeholder;

	if (["text", "string"].indexOf($type) >= 0) {
		$($jqElemRoot).append(getTextInputItemTemplate($name, $placeholder, $value || "", $label, $description, $hidden));
	} else if (["number", "integer", "int", "decimal"].indexOf($type) >= 0) {
		const $max = $element.max;
		const $min = $element.min;

		$($jqElemRoot).append(getNumberInputItemTemplate($name, $placeholder, $value || null, $label, $description, $max, $min, $hidden));
	} else if (["checkbox", "boolean", "bool"].indexOf($type) >= 0) {
		const $onChange = $element.on_change;

		var chckEl = $(getCheckBoxItemTemplate($name, $value === '1', $label, $description, $hidden));

		if ($onChange) {
			var inputEl = chckEl.find('input');

			PendingElementsToToggleState.push(
				{
					state: ($value === '1' ? "on_checked" : "on_unchecked"),
					elements: $onChange
				}
			)

			// listen to changes
			inputEl.change(function (e) {
				if (e.target.checked) {
					toggleStateElements("on_checked", $onChange);
				} else {
					toggleStateElements("on_unchecked", $onChange);
				}
			});
		}

		$($jqElemRoot).append(chckEl);
	}
}

function addGroups($groups, $values, $jqElemRoot, $translations) {
	[...$groups].forEach(group => {
		var groupEl = $(getGroupTemplate(_(group.name), (group.id || "").toLowerCase() || group.name.toLowerCase()));

		if (group.groups) {
			addGroups(group.groups, $values, groupEl, $translations);
		}

		group.elements.forEach(el => {
			const $el_name = el.target;
			appendTemplateElement(groupEl, $values[$el_name], el, $translations[$el_name].label, $translations[$el_name].description);
		});
		$jqElemRoot.append(groupEl);
	});
}

function toggleStateElements($selector, $elements) {
	[...$elements].forEach(el => {
		var $inputs = document.querySelectorAll(`#${el.id.toLowerCase()} input`) || [];
		[...$inputs].forEach(input => {
			input.removeAttribute("enabled");
			input.removeAttribute("disabled");
			input.setAttribute(el[$selector], "");
		});
	});
}

function selectCameraROI($ev, $cameraIndex) {
	$($ev.target).addClass("is-loading");

	frameRequestOrigin = "roi";

	var cam = document.querySelector(`#camera-${$cameraIndex}`);

	var rotation = parseInt(cam.querySelector(`input[name="rotation"]`).value);
	var url = cam.querySelector(`input[name="url"]`).value || cameras[$cameraIndex].url;

	sendObj('get_camera_frame', { index: $cameraIndex, rotation, url });

	unfinishedRequests["get_camera_frame"] = function () {
		setTimeout(function () {
			$($ev.target).removeClass("is-loading");
		}, 500);
	}
}

function selectCameraIgnoredAreas($ev, $cameraIndex) {
	$($ev.target).addClass("is-loading");

	frameRequestOrigin = "ig-areas";

	var cam = document.querySelector(`#camera-${$cameraIndex}`);

	var rotation = parseInt(cam.querySelector(`input[name="rotation"]`).value);

	var roi = cam.querySelector('input[name="roi"]').value;

	var url = cam.querySelector(`input[name="url"]`).value || cameras[$cameraIndex].url;

	var parsedRoi = stringToRoi(roi);
	if (parsedRoi) {
		$(cnvAreas.canvas).attr("width", parsedRoi[2])
		$(cnvAreas.canvas).attr("height", parsedRoi[3]);
	}

	sendObj('get_camera_frame', { index: $cameraIndex, rotation, url, roi });

	unfinishedRequests["get_camera_frame"] = function () {
		setTimeout(function () {
			$($ev.target).removeClass("is-loading");
		}, 500);
	}
}

function addNewCamera($ev) {

	sendObj("get_new_camera", {});
}

function deleteCamera($ev, $cameraIndex) {
	if (confirm(_("delete camera") + " " + cameras[$cameraIndex].cameraname + "?")) {
		$($ev.target).addClass("is-loading");

		lastConfigurationActive = "program";
		cameras.splice($cameraIndex, 1);
		document.getElementById('camera-' + $cameraIndex).remove();
		document.getElementById('tab-camera-' + $cameraIndex).remove();

		document.getElementById('tab-program').click();
	}
}

function saveCameraROI($ev, save) {
	var camindex = $('#modal-roi')[0].dataset["index"];
	var camera = document.querySelector('#camera-' + camindex);

	if (save) {
		var roiInput = camera.querySelector('input[name="roi"]');
		roiInput.value = cnvRoi.roi;
	}

	cnvRoi.x = cnvRoi.y = 0;

	camera.querySelector('.button-select-camera-roi').classList.remove('is-loading');
	$('#modal-roi').toggleClass('is-active');
}

function saveCameraIgarea($ev, save) {
	var camindex = $('#modal-igarea')[0].dataset["index"]; // $('#modal-igarea').data("index"); <-- bugged
	var camera = document.querySelector('#camera-' + camindex);

	if (save) {
		var igAreaInput = camera.querySelector('input[name="ignoredareas"]');
		igAreaInput.value = cnvAreas.areasString;
	} else {
		cnvAreas.areas = [];
	}

	cnvAreas.x = cnvAreas.y = 0;

	camera.querySelector('.button-select-camera-ignored-areas').classList.remove('is-loading');
	$('#modal-igarea').toggleClass('is-active');
}

function onResize() {
	var bounds = cnvRoi.canvas.getBoundingClientRect()
	cnvRoi.x = bounds.left;
	cnvRoi.y = bounds.top;

	var bounds = cnvAreas.canvas.getBoundingClientRect()
	cnvAreas.x = bounds.left;
	cnvAreas.y = bounds.top;
}

function getRectangleDimensions($p1, $p2) {
	const lt = { x: Math.round(Math.min($p1.x, $p2.x)), y: Math.round(Math.min($p1.y, $p2.y)) }
	const br = { x: Math.round(Math.max($p1.x, $p2.x)), y: Math.round(Math.max($p1.y, $p2.y)) }

	const width = br.x - lt.x;
	const heigth = br.y - lt.y;

	return [lt, width, heigth];
}

document.addEventListener('DOMContentLoaded', () => {

	// Get all "navbar-burger" elements
	const $navbarBurgers = Array.prototype.slice.call(document.querySelectorAll('.navbar-burger'), 0);

	// Check if there are any navbar burgers
	if ($navbarBurgers.length > 0) {

		// Add a click event on each of them
		$navbarBurgers.forEach(el => {
			el.addEventListener('click', () => {

				// Get the target from the "data-target" attribute
				const target = el.dataset.target;
				const $target = document.getElementById(target);

				// Toggle the "is-active" class on both the "navbar-burger" and the "navbar-menu"
				el.classList.toggle('is-active');
				$target.classList.toggle('is-active');
			})
		})
	}

	$('#toggle-push').attr("checked", SEND_PUSH_NOTIFICATIONS);
	$('#toggle-notification-sound').attr("checked", PLAY_SOUND_NOTIFICATION);

	// Update notifications time
	setInterval(() => {
		[...document.querySelectorAll('.hour')].forEach(el => {
			el.innerHTML = moment(el.dataset.date, "MMMM Do YYYY, h:mm:ss a").fromNow();
		});
	}, 10 * 1000);

	document.querySelector('#modal-roi .modal-content').addEventListener('scroll', onResize, false);
	document.querySelector('#modal-igarea .modal-content').addEventListener('scroll', onResize, false);

	// dropdowns
	var dropdown_file = document.querySelector('#dropdown-file');
	dropdown_file.addEventListener('click', function (event) {
		event.stopPropagation();
		dropdown_file.classList.toggle('is-active');
	});

	var dropdown_lang = document.querySelector('#dropdown-language');
	dropdown_lang.addEventListener('click', function (event) {
		event.stopPropagation();
		dropdown_lang.classList.toggle('is-active');
	});
	
	[...dropdown_lang.querySelector('.dropdown-content').children].forEach(el => {
		if (el.dataset.lang != CURRENT_LANG)
			el.classList.remove('is-active');
		else
			el.classList.add('is-active');
	});

	moment.locale(CURRENT_LANG);

	// set ROI canvas listeners
	cnvRoi.canvas = document.querySelector('#modal-roi canvas');
	cnvRoi.ctx = cnvRoi.canvas.getContext('2d');

	$(function () { // change scope to use some custom move / onclick / ... functions that may repeat
		// Mouse or touch moved
		function move(e) {
			e.preventDefault();
			e = (e.touches || [])[0] || e;
			if (cnvRoi.clickPressed) {
				var image = new Image();
				image.onload = function () {
					cnvRoi.ctx.drawImage(image, 0, 0);

					const x1 = e.clientX - cnvRoi.x;
					const y1 = e.clientY - cnvRoi.y;

					cnvRoi.p2 = { x: x1, y: y1 };

					const x0 = cnvRoi.p1.x;
					const y0 = cnvRoi.p1.y;
					const width = x1 - x0;
					const heigth = y1 - y0;

					cnvRoi.ctx.strokeRect(x0, y0, width, heigth);
				};

				image.src = "data:image/jpg;base64," + cnvRoi.lastImage;
			}
		}

		cnvRoi.canvas.addEventListener("mousemove", move, false);
		cnvRoi.canvas.addEventListener("touchmove", move, false);

		// Click or touch pressed
		function pressed(e) {
			e.preventDefault();
			console.log(e);
			e = (e.touches || [])[0] || e;
			cnvRoi.clickPressed = true;
			const x = e.clientX - cnvRoi.x;
			const y = e.clientY - cnvRoi.y;

			cnvRoi.p1 = { x, y };
		}

		cnvRoi.canvas.addEventListener("mousedown", pressed, false);
		cnvRoi.canvas.addEventListener("touchstart", pressed, false);

		// Click or touch released
		function relesed(e) {
			cnvRoi.clickPressed = false;

			const [lt, width, heigth] = getRectangleDimensions(cnvRoi.p1, cnvRoi.p2);

			cnvRoi.roi = `[${lt.x},${lt.y}],[${width}, ${heigth}]`;
			e.preventDefault();
		}

		cnvRoi.canvas.addEventListener("mouseup", relesed, false);
		cnvRoi.canvas.addEventListener("touchend", relesed, false);
	});

	/* CANVAS IGNORED AREAS LISTENERS */
	$(function () {  // change scope to use some custom move / onclick / ... functions that may repeat
		// set ROI canvas listeners
		cnvAreas.canvas = document.querySelector('#modal-igarea canvas');
		cnvAreas.ctx = cnvAreas.canvas.getContext('2d');

		// Mouse or touch moved
		function move(e) {
			e.preventDefault();
			e = (e.touches || [])[0] || e;
			if (cnvAreas.clickPressed) {
				var image = new Image();
				image.onload = function () {
					// check again... there is alot of this calls at the same time and causes problems
					if (cnvAreas.clickPressed) {
						cnvAreas.ctx.drawImage(image, 0, 0);

						cnvAreas.areas.forEach(area => {
							cnvAreas.ctx.strokeStyle = area.color;
							cnvAreas.ctx.strokeRect(area.lt.x, area.lt.y, area.width, area.heigth);
						});

						const x1 = e.clientX - cnvAreas.x;
						const y1 = e.clientY - cnvAreas.y;

						cnvAreas.current.p2 = { x: x1, y: y1 };

						const x0 = cnvAreas.current.p1.x;
						const y0 = cnvAreas.current.p1.y;
						const width = x1 - x0;
						const heigth = y1 - y0;

						cnvAreas.ctx.strokeStyle = cnvAreas.current.color;

						cnvAreas.ctx.strokeRect(x0, y0, width, heigth);
					}
				};

				image.src = "data:image/jpg;base64," + cnvAreas.lastImage;
			}
		}

		cnvAreas.canvas.addEventListener("mousemove", move, false);
		cnvAreas.canvas.addEventListener("touchmove", move, false);

		// Click or touch pressed
		function pressed(e) {
			e.preventDefault();
			e = (e.touches || [])[0] || e;
			cnvAreas.lastClick = performance.now();
			cnvAreas.clickPressed = true;
			const x = e.clientX - cnvAreas.x;
			const y = e.clientY - cnvAreas.y;

			cnvAreas.current.color = cnvAreas.colors[getRandomArbitrary(0, cnvAreas.colors.length)];

			cnvAreas.current.p1 = cnvAreas.current.p2 = { x, y };
		}

		cnvAreas.canvas.addEventListener("mousedown", pressed, false);
		cnvAreas.canvas.addEventListener("touchstart", pressed, false);

		// Click or touch released
		function relesed(e) {
			cnvAreas.clickPressed = false;
			var time = (performance.now() - cnvAreas.lastClick) / 1000;

			const [lt, width, heigth] = getRectangleDimensions(cnvAreas.current.p1, cnvAreas.current.p2);

			// if it's dimesion is small enough and was a quick click then delete it, else save it

			if (time < 2 && time > 0 && width > -2 && width < 2 && heigth > -2 && heigth < 2) {
				const p = cnvAreas.current.p1;
				cnvAreas.areas.forEach((area, index, object) => {
					if (p.x >= area.lt.x && p.x <= area.lt.x + area.width &&
						p.y >= area.lt.y && p.y <= area.lt.y + area.heigth) {
						object.splice(index, 1);
					}
				});

				// draw areas
				var image = new Image();
				image.onload = function () {
					cnvAreas.ctx.drawImage(image, 0, 0);

					cnvAreas.areas.forEach(area => {
						cnvAreas.ctx.strokeStyle = area.color;
						cnvAreas.ctx.strokeRect(area.lt.x, area.lt.y, area.width, area.heigth);
					});
				};

				image.src = "data:image/jpg;base64," + cnvAreas.lastImage;
			} else {
				cnvAreas.areas.push({ lt, width, heigth, color: cnvAreas.current.color });

				cnvAreas.areasString = "";
				cnvAreas.areas.forEach(area => {
					cnvAreas.areasString += `[${area.lt.x},${area.lt.y}],[${area.width}, ${area.heigth}],`;
				});

				cnvAreas.areasString = cnvAreas.areasString.substring(0, cnvAreas.areasString.length - 1);

				cnvAreas.current.p1 = { x: 0, y: 0 };
				cnvAreas.current.p2 = { x: 0, y: 0 };
			}
			e.preventDefault();
		}

		cnvAreas.canvas.addEventListener("mouseup", relesed, false);
		cnvAreas.canvas.addEventListener("touchend", relesed, false);
	});
});

window.addEventListener("focus", function (event) {
	setTimeout(() => {
		if (document.visibilityState == "visible") {
			currentNumberNotificationsWindowTitle = 0;
			document.title = appTitle;
		}
	}, 250);
}, false);

// Helper function that updates the current index to the previous and calls a function to change the notification displayed
function previousNotification() {
	var $i = notificationPaginator.index > 0 ? notificationPaginator.index - 1 : notificationPaginator.elements.length - 1;
	changeCurrentElementNotification($i);
	window.scrollTo(0,document.body.scrollHeight);
}

// Helper function that updates the current index to the next one and calls a function to change the notification displayed
function nextNotification() {
	var $i = notificationPaginator.index < notificationPaginator.elements.length - 1 ? notificationPaginator.index + 1 : 0;
	changeCurrentElementNotification($i);
	window.scrollTo(0,document.body.scrollHeight);}

// updates the number of notifications shown on the buttons to change notification
function updateNotificationsNumberPaginator() {
	$('.notification-next-left').text(notificationPaginator.elements.length - 1 - notificationPaginator.index);
	$('.notification-previous-left').text(notificationPaginator.index);
}

// removes the notifications elements from the container and append the requested
function changeCurrentElementNotification($i) {
	var $not = $('#notifications-content');
	
	// remove all the soruces from the images of all childrends of the notification container
	[...$not[0].children].forEach(el => {
		var img = el.getElementsByTagName("img")[0];
		if (img) {
			img.dataset.src = img.src;
			img.src = "";
		}

		var videos = el.getElementsByTagName("video");
		if (videos.length > 0) {
			[...videos].forEach(vid => {
				vid.dataset.src = vid.src;
				vid.src = "";
			});
		}
	});

	// remove the childrends from the tree
	$not.empty();
	
	var el = notificationPaginator.elements[$i];

	if (el.id) el = el.root[0];

	const loadVideos = function () {
		var videos = el.getElementsByTagName("video");
		if (videos.length > 0) {
			[...videos].forEach(vid => {
				vid.src = vid.dataset.src;
			});
		}
	};

	if (el.getElementsByTagName("img").length > 0) {
		var img = el.getElementsByTagName("img")[0];
		if (img) {
			// set src for the image elements
			img.src = img.dataset.src;
			
			// set src for the video elements
			img.onload = loadVideos
		} else {
			loadVideos();
		}
	}

	// append notification
	$not.append(el);

	// update current index
	notificationPaginator.index = $i;

	// update before/after notifications left
	updateNotificationsNumberPaginator();
}

function getRandomArbitrary(min, max) {
	return Math.round(Math.random() * (max - min) + min);
}
