function locationSuccess(pos) {
  var coordinates = pos.coords;
  console.log(coordinates.latitude);
  console.log(coordinates.longitude);
  
  var req = new XMLHttpRequest();
  req.open('GET', 'http://344fbbb2.ngrok.io/?longitude=' + coordinates.longitude + '&latitude=' + coordinates.latitude, true);
  req.send(null);

}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 30000
};

Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  console.log(e.type);
});