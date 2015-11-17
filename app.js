var config = require('./config.sample.json')
var url = require('url');

var Uber = require('node-uber');

var TwilioAccountSID = config["TWILIO_ACCOUNT_SID"]
var TwilioAuthToken = config["TWILIO_TOKEN"]
var number = config["TWILIO_NUMBER"]

var uberClientId = config["UBER_CLIENT_ID"]
var uberClientSecret = config["UBER_CLIENT_SECRET"]
var uberServerToken = config["UBER_SERVER_TOKEN"]

var client = require('twilio')(TwilioAccountSID, TwilioAuthToken);

var expressPort = config["PORT"];
var express = require('express');
var app = express();

var uberXId = "28fc0b4e-3f7f-431a-889e-84e9cb4192e1";

var uber = new Uber({
	client_id: uberClientId,
	client_secret: uberClientSecret,
	server_token: uberServerToken,
	redirect_uri: 'http://localhost:' + expressPort + '/auth',
	name: 'HappyHour'
});

// uber.products.list({ latitude: 43.472547, longitude: -80.539878 }, function (err, res) {
//   if (err) console.error(err);
//   else console.log(res);
// });
app.set('port', process.env.PORT || 8001);

function sendMessage(toNum, bodyText){

	console.log("Sending to", toNum, ":", bodyText)

	client.messages.create({
		body: bodyText,
		to: toNum,
		from: number,
		}, function(err, message) {
			if(err){
				console.log(err)
			}
			else{
				// console.log(message)
			}
	});
}

app.get('/', function (req, res) {
	var url_parts = url.parse(req.url, true);
	var lat = url_parts.query.latitude;
	var lon = url_parts.query.longitude;
	console.log(lat);
	console.log(lon);

	var lat_home = 43.500057;
	var lon_home = -80.556927;

	uber.estimates.price({
      start_latitude: lat, start_longitude: lon,
      end_latitude: lat_home, end_longitude: lon_home
    }, function (err, res) {
      if (err) console.error(err);
      else {
      	var est_cost = res.prices[0].estimate;
      	var distance = res.prices[0].distance;
      	var duration = res.prices[0].duration / 60;

      	sendMessage("+15105658237", "Hello from HappyHour! Your Uber will be here in " + duration + " minutes. Your house is " + distance + " miles away; estimated cost of this trip is " + est_cost + ".");

      }
    });

});


var server = app.listen(app.get('port'), function() {
  console.log('Our app is running on port %d', server.address().port);
});

app.get("/verify", function(req, res){
	res.redirect("http://login.uber.com/oauth/authorize?response_type=code&client_id=" + uberClientId + "&scope=profile%20request")
})

app.get("/auth", function(req, res){
	var authCode = req.query.code
	console.log(authCode)
})
