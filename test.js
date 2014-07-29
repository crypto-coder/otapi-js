// set up ========================
var otapi  = require('./build/Release/node_otapi');


/*
var express  = require('express');
var app      = express(); 			// create our app w/ express
var mongoose = require('mongoose'); 		// mongoose for mongodb

// configuration =================
var mongoDBConnectionString = 'mongodb://mongodb:mongodb@127.0.0.1:27017/uwO3mypu';
mongoose.connect(mongoDBConnectionString); 	// connect to mongoDB database on modulus.io

app.configure(function() {
	app.use(express.static(__dirname + '/public')); // set the static files location /public/img will be /img for users
	app.use(express.logger('dev')); 	// log every request to the console
	app.use(express.bodyParser()); 		// pull information from html in POST
});

// listen (start app with node server.js) ======================================
app.listen(8080);
console.log("App listening on port 8080");
*/






console.log(otapi.startOTAPI('IirfVWfx09PmHAznlJUR93jvFZmSZ6Wh5fYHkNERxKj', 'password'));
console.log('issuer = ' + otapi.getAccountBalance('k5YaRDf1sJOpQzxmE2nTQJNczmi7GWtxwVmoBeKtlYO'));
console.log('comptroller = ' + otapi.getAccountBalance('aa2Tsh213OUm3fUnRb3W0DKLbU5owDaGZNH5nI6ZmQW'));
//console.log(otapi.mainUserID);
//console.log(otapi.mainServerID);
//console.log(otapi.getAccountList());
console.log(otapi.transferAssets('k5YaRDf1sJOpQzxmE2nTQJNczmi7GWtxwVmoBeKtlYO', 'aa2Tsh213OUm3fUnRb3W0DKLbU5owDaGZNH5nI6ZmQW', 100, 'test transfer'));
console.log('issuer = ' + otapi.getAccountBalance('k5YaRDf1sJOpQzxmE2nTQJNczmi7GWtxwVmoBeKtlYO'));
console.log('comptroller = ' + otapi.getAccountBalance('aa2Tsh213OUm3fUnRb3W0DKLbU5owDaGZNH5nI6ZmQW'));
console.log(otapi.stopOTAPI());



//var http = require('http');
/*
http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.end('Hello World\n');
}).listen(1337, '127.0.0.1');
**/
//console.log('Server running at http://127.0.0.1:1337/');



