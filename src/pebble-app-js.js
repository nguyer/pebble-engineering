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
		}, 1800000);
	}
);

function getTemp(lat, lon, callback) {
	var req = new XMLHttpRequest();
	var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + lat + '&lon=' + lon + '&units=imperial';
	console.log(url);
	req.open('GET', url, true);
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
			if(req.status == 200) {
				var res = JSON.parse(req.responseText);
				var temp = res.main.temp;
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
	console.log(temp);
	var transactionId = Pebble.sendAppMessage( { 'TEMPERATURE': String(temp) },
  function(e) {
    console.log('Successfully delivered message with transactionId='
      + e.data.transactionId);
  },
  function(e) {
    console.log('Unable to deliver message with transactionId='
      + e.data.transactionId
      + ' Error is: ' + e.error.message);
  }
);
}