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
			<button class="button button-select-camera-roi" onclick="selectCameraROI(event, ${i})">Select camera region of interest</button>
			<button class="button button-select-camera-ignored-areas" onclick="selectCameraIgnoredAreas(event, ${i})">Select camera ignored areas</button>
			<button class="button is-danger" onclick="deleteCamera(event, ${i})">Delete camera</button>
		</div>
		</div>
	</footer>
</div>`;

const getProgramContainerTemplate = () => `
<div class="card" id="program">
	<header class="card-header">
		<p class="card-header-title">
			Program configuration
		</p>
	</header>
	<div class="card-content program-config-content">

	</div>
</div>`;

const getTextInputItemTemplate = ($name, $placeholder, $value, $label, $tooltip, $hidden) => `
<div class="card-content-item ${$hidden && 'is-hidden'}">
	<label for="${$name}" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="${$tooltip}">${$label}</label>
	<input class="input" name="${$name}" type="text" placeholder="${$placeholder}" value="${$value}">
</div>`;

const getNumberInputItemTemplate = ($name, $placeholder, $value, $label, $tooltip, $max, $min, $hidden) => `
<div class="card-content-item ${$hidden && 'is-hidden'}">
	<label for="${$name}" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="${$tooltip}">${$label}</label>
	<input class="input" name="${$name}" type="number" placeholder="${$placeholder}" ${$min != null && 'min="'+$min +'"'} ${$max != null && 'max="'+$max +'"'} ${$value && "value='"+$value +"'"}>
</div>`;

const getCheckBoxItemTemplate = ($name, $checked, $label, $tooltip, $hidden) => `
<div class="card-content-item ${$hidden && 'is-hidden'}">
	<label class="checkbox" class="has-tooltip-multiline has-tooltip-text-centered has-tooltip-right" data-tooltip="${$tooltip}">
		<input type="checkbox" name="${$name}" ${$checked && 'checked'}>
		${$label}
	</label>
</div>`;

const getNotificationTemplate = (type, text) => `
<div class="box">
${(type == "image" && 
	`<figure class="image">
		<img src="${text}" alt="Image">
	</figure>`
) || ""}
${(type == "text" && `<h3 class="subtitle is-3">${text}</h3>`) || ""}
<h6 class="subtitle is-6 hour" data-date="${moment().format('MMMM Do YYYY, h:mm:ss a')}">now</h6>
</div>`;

const getTabTemplate = (i, camName) => `
<li data-config="camera-${i}">
	<a><span>${camName}</span></a>
</li>`

var FILE_PATH = ""; // file that the user requested at the start
var RECOGNIZE_RUNNING = false;
var IS_NOTIFICATION_PAGE = false; // showing notitifcation page
var ws;
var lastConfigurationActive = "program"; // id of the element
var cameras = []; // actual cameras as an dictionary
var unfinishedRequests = {}; 	// dict of unfinished request where key is the request and key a function to
								// execute when we receive it.
var cnvRoi = {
	canvas: null,
	ctx: null,
	lastImage: "",
	clickPressed: false,
	p1: {x: 0, y: 0}, // lt or rb
	p2: {x: 0, y: 0}, // rb or lt
	x: 0, // canvas position in x
	y: 0, // canvas position in y (with scrollbar)
	roi: ""
}

var notificationPaginator = {index: 0, elements:[]}; // DOM notification elements

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
		} 
		
		if (data.hasOwnProperty("configuration_file")) {
			console.log(data);
			setTimeout(() => $('#modal-file').removeClass('is-active'), 50);

			//  Add accordion program and cameras
			var headers = getHeadersFromStringConfig(data["configuration_file"]);

			cameras = headers.cameras;
			
			getElementsTranslations().then(res => {				
				const [elements, translations] = res;

				/// ---- ADD PROGRAM ITEM			
				var programEl = $(getProgramContainerTemplate());
				var programContent = $(programEl).children('.program-config-content');
				
				addTemplateElements(programContent, headers.program, elements.program, translations.en);

				$('#configurations').append(programEl);

				/// ---- END PROGRAM ITEM

				/// ==== ADD CAMERAS CONFIGURATIONS
				var tabs = document.querySelector('.tabs ul');
				headers["cameras"].forEach((val, i) => {
					// add tab
					$(tabs).append(getTabTemplate(i, val["cameraname"]));

					// get camera root container
					var camEl = $(getCameraContainerTemplate(i, val));
					var camConten = $(camEl).children('.camera-config-content');
					
					// add each input element
					addTemplateElements(camConten, val, elements.camera, translations.en);

					// add to configurations container
					$('#configurations').append(camEl);
				});				
				/// ==== END ADD CAMERAS CONFIGURATIONS

				// set listeners tabs
				tabs = [...document.querySelector('.tabs ul').querySelectorAll('li')];
				tabs.forEach(el => {
						el.onclick = function () {
							document.getElementById(lastConfigurationActive).classList.add('is-hidden');
							
							tabs.forEach(el => el.classList.remove('is-active'))
						
							el.classList.add('is-active');
							
							document.getElementById(el.dataset.config).classList.remove('is-hidden');
		
							lastConfigurationActive = el.dataset.config;
						}
					});
			});
		} 
		
		if (data.hasOwnProperty("recognize_state_changed")) {
			$('#button-toggle-recognize').removeClass('is-loading');
			RECOGNIZE_RUNNING = data["recognize_state_changed"];
			changeRecognizeStatusElements(RECOGNIZE_RUNNING);
		}
		
		if (data.hasOwnProperty("new_notification")) {
			var ob = data["new_notification"];
			console.log("Notification: ", ob);
			if (ob["type"] != "sound") {
				var not_content = document.getElementById('notifications-content');
				var $not = $(getNotificationTemplate(ob["type"], ob['content']));
								
				notificationPaginator.elements.push($not[0]);

				changeCurrentElementNotification(notificationPaginator.elements.length - 1);

				Push.create("Alert!", {
					body: "...",
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

		if (data.hasOwnProperty('request_reply')) {
			var ob = data["request_reply"];
			if (ob["status"] == "ok") {
				$('#notification').removeClass('is-danger')
								  .addClass('is-success');				
			} else if (ob["status"] == "error") {
				$('#notification').removeClass('is-success')
								  .addClass('is-danger');
			}

			$('#notification').removeClass('is-hidden')
			$('#notification span').text(ob["message"]);

			setTimeout(() => $('#notification').addClass('is-hidden'), 6000);

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

			var modal = document.querySelector('#modal-roi');
			
			modal.classList.add('is-active');
			
			modal.dataset.index = index;

			var image = new Image();
			image.onload = function() {
				cnvRoi.ctx.drawImage(image, 0, 0);

				cnvRoi.ctx.strokeStyle = "Red"; 
				cnvRoi.ctx.lineWidth = 5;

				var camera = document.querySelector('#camera-' + index);
				var roiInput = camera.querySelector('input[name="roi"]');
				cnvRoi.roi = roiInput.value;
				if (roiInput.value.length > 0) {
					var numbers = cnvRoi.roi.match(/\d+/g).map(i => parseInt(i));
					if (numbers.length === 4)
						cnvRoi.ctx.strokeRect(numbers[0], numbers[1], numbers[2], numbers[3]);
				}

				onResize();
			};
			image.src = "data:image/jpg;base64," + frame;
			
			cnvRoi.lastImage = frame;
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
		$(this).toggleClass('is-loading');
		ws.send("change_recognize_state " + !RECOGNIZE_RUNNING);
	});

	$('#button-just-notifications').click(function () {
		$('#configuration-page').remove();
		$('#button-current-page').remove();
		togglePage();
		$('#modal-file').toggleClass('is-active');
		Notification.requestPermission();
	});
});


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

function getElementsTranslations() {
	const objectKeysToLowerCase = obj => Object.fromEntries(
		Object.entries(obj).map(([k, v]) => [k.toLowerCase(), v])
	)

	return new Promise(resolve => {
		fetch('/elements.json')
			.then(elements => elements.json())
			.then(elements => {
			elements.camera = objectKeysToLowerCase(elements.camera)
			elements.program = objectKeysToLowerCase(elements.program)

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

function addTemplateElements(jqElemRoot, values, elements, translations) {
	Object.entries(elements).forEach(el => {
		const $name = el[0];
		const $type = el[1].type;
		const $hidden = el[1].hidden;
		const $placeholder = el[1].placeholder;
		const $label = translations[$name].label;
		const $tooltip = translations[$name].description;
		
		if (["text", "string"].indexOf($type) >= 0) {
			$(jqElemRoot).append(getTextInputItemTemplate($name, $placeholder, values[$name] || "", $label, $tooltip, $hidden));
		} else if (["number", "integer", "int", "decimal"].indexOf($type) >= 0) {
			const $max = el[1].max;
			const $min = el[1].min;

			$(jqElemRoot).append(getNumberInputItemTemplate($name, $placeholder, values[$name] || null, $label, $tooltip, $max, $min, $hidden));
		} else if (["checkbox", "boolean", "bool"].indexOf($type) >= 0) {
			$(jqElemRoot).append(getCheckBoxItemTemplate($name, values[$name] === '1', $label, $tooltip, $hidden));
		}
	});
}

function selectCameraROI($ev, $cameraIndex) {
	$($ev.target).addClass("is-loading");

	var rotation = parseInt(document.querySelector(`#camera-${$cameraIndex} input[name="rotation"]`).value);
	
	ws.send(`get_camera_frame ${$cameraIndex} ${rotation} ${cameras[$cameraIndex].url}`);

	unfinishedRequests["get_camera_frame"] = function () {
		setTimeout(function (){
		$($ev.target).removeClass("is-loading");
		}, 500);
	}
}

function selectCameraIgnoredAreas($ev, $cameraIndex) {
	$($ev.target).addClass("is-loading");
}

function deleteCamera($ev, $cameraIndex) {
	$($ev.target).addClass("is-loading");
}

function saveCameraROI($ev, save) {
	var camindex = $('#modal-roi').data("index");
	var camera = document.querySelector('#camera-' + camindex);

	if (save) {
		var roiInput = camera.querySelector('input[name="roi"]');
		roiInput.value = cnvRoi.roi;
	}

	cnvRoi.x = cnvRoi.y = 0;

	camera.querySelector('.button-select-camera-roi').classList.remove('is-loading');	
	$('#modal-roi').toggleClass('is-active');
}

function onResize() {
	var bounds = cnvRoi.canvas.getBoundingClientRect()
	cnvRoi.x = bounds.left;
	cnvRoi.y = bounds.top;
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
			})
		})
	}

	document.querySelectorAll('.notification .delete').forEach(($delete) => {
		var $notification = $delete.parentNode;

		$delete.addEventListener('click', () => {
			$($notification).toggleClass('is-hidden');
		});
	});

	// Update notifications time
	setInterval(() => {
		[...document.querySelectorAll('.hour')].forEach(el => {
			el.innerHTML = moment(el.dataset.date, "MMMM Do YYYY, h:mm:ss a").fromNow();
		});
	}, 10 * 1000);

	document.querySelector('#modal-roi .modal-content').addEventListener('scroll', onResize, false);

	// set ROI canvas listeners
	cnvRoi.canvas = document.querySelector('#modal-roi canvas');
	cnvRoi.ctx = cnvRoi.canvas.getContext('2d');
	
	// Mouse or touch moved
	function move (e) {
		e.preventDefault();		
		e = (e.touches || [])[0] || e;
		if (cnvRoi.clickPressed) {
			var image = new Image();
			image.onload = function() {
				cnvRoi.ctx.drawImage(image, 0, 0);

				const x1 = e.clientX - cnvRoi.x;
				const y1 = e.clientY - cnvRoi.y;

				cnvRoi.p2 = {x: x1, y: y1};

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
	function pressed (e) {
		e.preventDefault();
		console.log(e);
		e = (e.touches || [])[0] || e;
		cnvRoi.clickPressed = true;
		const x = e.clientX - cnvRoi.x;
		const y = e.clientY - cnvRoi.y;

		cnvRoi.p1 = {x,y};
	}

	cnvRoi.canvas.addEventListener("mousedown", pressed, false);
	cnvRoi.canvas.addEventListener("touchstart", pressed, false);

	// Click or touch released
	function relesed (e) {
		cnvRoi.clickPressed = false;

		const lt = {x: Math.round(Math.min(cnvRoi.p1.x, cnvRoi.p2.x)), y: Math.round(Math.min(cnvRoi.p1.y, cnvRoi.p2.y))}
		const br = {x: Math.round(Math.max(cnvRoi.p1.x, cnvRoi.p2.x)), y: Math.round(Math.max(cnvRoi.p1.y, cnvRoi.p2.y))}

		const width = br.x - lt.x;
		const heigth = br.y - lt.y;
		
		cnvRoi.roi = `[${lt.x},${lt.y}],[${width}, ${heigth}]`;
		e.preventDefault();
	}

	cnvRoi.canvas.addEventListener("mouseup", relesed, false);
	cnvRoi.canvas.addEventListener("touchend", relesed, false);
});

function previousNotification(){
	var $i = notificationPaginator.index > 0 ? notificationPaginator.index  - 1 : 0;
	changeCurrentElementNotification($i);
}

function nextNotification(){
	var $i = notificationPaginator.index < notificationPaginator.elements.length - 1 ? notificationPaginator.index + 1 : 0;
	changeCurrentElementNotification($i);
}

function changeCurrentElementNotification($i) {
	var $not = $('#notifications-content');
	$not.empty();

	$not.append(notificationPaginator.elements[$i]);
	notificationPaginator.index = $i;
	
	$('.notification-next-left').text(notificationPaginator.elements.length - 1 - notificationPaginator.index);
	$('.notification-previous-left').text(notificationPaginator.index);
}
	