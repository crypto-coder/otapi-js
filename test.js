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



otapi.mainUserID = 'ya1AQQmaWnuntmDnoOjCmKpPXhmGuVAfkPwrgSc3nlf';
otapi.mainServerID = 'y0ca6JVtYSZuj1etoABAsaNJsU2Kb35AjeQZyZ0YCCF';


console.log(otapi.startOTAPI('ya1AQQmaWnuntmDnoOjCmKpPXhmGuVAfkPwrgSc3nlf', 'password'));
console.log('issuer = ' + otapi.getAccountBalance('MtBJmjl6FXD51lnSAgENXwXh748BRe7j5TDd0vf8OXd'));
console.log('comptroller = ' + otapi.getAccountBalance('oYeRsNAzcf0IcuqNEVtCdvW0fmt5F3FRucTJcygMcUd'));
console.log(otapi.mainUserID);
console.log(otapi.mainServerID);

console.log('\n\n\n\n');
console.log('ACCOUNT ID LIST');
console.log(otapi.getAccountIDList());

console.log('\n\n\n\n');
console.log('NYM ID LIST');
console.log(otapi.getNymIDList());

console.log('\n\n\n\n');
console.log('NYM NAME LIST');
console.log(otapi.getNymNameList());

console.log('\n\n\n\n');
console.log('ASSET NAME LIST');
console.log(otapi.getAssetNameList());


console.log('\n\n\n\n');
var newNymID = otapi.createNewNym('test mama jama');
console.log(newNymID);

//console.log(otapi.transferAssets('k5YaRDf1sJOpQzxmE2nTQJNczmi7GWtxwVmoBeKtlYO', 'aa2Tsh213OUm3fUnRb3W0DKLbU5owDaGZNH5nI6ZmQW', 100, 'test transfer'));



console.log('issuer = ' + otapi.getAccountBalance('MtBJmjl6FXD51lnSAgENXwXh748BRe7j5TDd0vf8OXd'));
console.log('comptroller = ' + otapi.getAccountBalance('oYeRsNAzcf0IcuqNEVtCdvW0fmt5F3FRucTJcygMcUd'));
console.log(otapi.stopOTAPI());



//var http = require('http');
/*
http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.end('Hello World\n');
}).listen(1337, '127.0.0.1');
**/
//console.log('Server running at http://127.0.0.1:1337/');



