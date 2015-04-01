var inputEventListener = require('../build/Release/input_event_listener.node').inputEventListener;

inputEventListener("/dev/input/event0", function(event) {
	console.log(event);
});

setTimeout(require('../build/Release/input_event_listener.node').Destroy, 5000);