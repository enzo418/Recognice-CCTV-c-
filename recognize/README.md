# Recognize people on CCTV

# Requeriments	
	- Opencv >= 4.5 (was 3.0 before implementing yolov4 nn)

	## Compiling
	- jsoncpp-devel (To decode incoming messages from the telegram api)


# Todo
**1. Comment the code**
	
**2. Configuration:**
- Allow the user to set a messaging service like dweet.io

**3. Others**
- Add a flag that, if enabled, will make log every 30m the average change of pixels. Also the maximun and minimum. With this the user will be able to set the perfect threshold for each hor/time of the day.