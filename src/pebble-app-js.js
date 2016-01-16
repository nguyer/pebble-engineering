var openWeatherMapApiKey = '';
var units = 'imperial';

if (localStorage.units) {
	units = localStorage.units;
}

var locationOptions = {
  enableHighAccuracy: false,
  maximumAge: 10000,
  timeout: 10000
};

function locationSuccess(pos) {
	console.log('lat= ' + pos.coords.latitude + ' lon= ' + pos.coords.longitude);
	getTemp(pos.coords.latitude, pos.coords.longitude, function(err, temp) {
		if (err) {
			console.log(err);
		}
		else {
			console.log('Current temperature: ' + temp);
			sendTemp(temp);
		}
	});
}

function locationError(err) {
  console.log('location error (' + err.code + '): ' + err.message);
}

Pebble.addEventListener('ready',function(e) {
		// Request current position
		navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);

		setInterval(function() {
			navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
		}, 60 * 60000); // 60 minutes
	}
);

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://rawgit.com/nguyer/pebble-engineering/1.5/config/index.html';
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
	var configData = JSON.parse(decodeURIComponent(e.response));
	console.log('Configuration page returned: ' + JSON.stringify(configData));

	if (configData.units && configData.units !== units) {
		// update units
		units = configData.units;
		localStorage.units = configData.units;
		navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
	}

	var toggleDict = {};

	toggleDict.SHOW_NUMBERS = configData.show_numbers;
	toggleDict.SHOW_SECOND_HAND = configData.show_second_hand;
	toggleDict.SHOW_DATE = configData.show_date;
	toggleDict.SHOW_TEMPERATURE = configData.show_temperature;

	var colorDict = {};

	colorDict.COLOR_BACKGROUND = parseInt(configData.background_color.substring(0), 16);
	colorDict.COLOR_LABEL = parseInt(configData.label_color.substring(0), 16);
	colorDict.COLOR_HOUR_MARKS = parseInt(configData.hour_mark_color.substring(0), 16);
	colorDict.COLOR_MINUTE_MARKS = parseInt(configData.minute_mark_color.substring(0), 16);
	colorDict.COLOR_HOUR_HAND = parseInt(configData.hour_hand_color.substring(0), 16);
	colorDict.COLOR_MINUTE_HAND = parseInt(configData.minute_hand_color.substring(0), 16);
	colorDict.COLOR_SECOND_HAND = parseInt(configData.second_hand_color.substring(0), 16);

	Pebble.sendAppMessage(toggleDict, function() {
		console.log('Send toggles successful: ' + JSON.stringify(toggleDict));

		Pebble.sendAppMessage(colorDict, function() {
			console.log('Send colors successful: ' + JSON.stringify(colorDict));
		}, function() {
			console.log('Send failed!');
		});

	}, function() {
		console.log('Send failed!');
	});




});

function getTemp(lat, lon, callback) {
	var req = new XMLHttpRequest();
	var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + lat + '&lon=' + lon + '&units=' + units + '&APPID=';
	console.log(url);
	url += openWeatherMapApiKey;
	req.open('GET', url, true);
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
			if(req.status == 200) {
				var res = JSON.parse(req.responseText);
				var temp = res.main.temp;

				if (units == 'si') {

				}

				callback(null, temp);
			}
			else {
				callback('Error');
			}
		}
	};

	req.send(null);
}

function sendTemp(temp) {
	temp = Math.round(temp);
	Pebble.sendAppMessage( { 'TEMPERATURE': temp },
  function(e) {
    console.log('Successfully delivered message with transactionId='
      + e.data.transactionId);
  },
  function(e) {
    console.log('Unable to deliver message with transactionId='
      + e.data.transactionId
      + ' Error is: ' + e.error.message);
  });
}
