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











function startup(){
  console.log(otapi.startOTAPI('password'));
  //console.log('issuer = ' + otapi.getAccountBalance('MtBJmjl6FXD51lnSAgENXwXh748BRe7j5TDd0vf8OXd'));
  //console.log('comptroller = ' + otapi.getAccountBalance('oYeRsNAzcf0IcuqNEVtCdvW0fmt5F3FRucTJcygMcUd'));
  //console.log(otapi.mainUserID);
  //console.log(otapi.mainServerID);
}

function listEverything(){
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
  console.log('ASSET ID LIST');
  allAssetID = otapi.getAssetIDList();
  console.log(allAssetID);

  console.log('\n\n\n\n');
  console.log('ASSET NAME LIST');
  console.log(otapi.getAssetNameList());
}



function createUserAndAccount(){
  console.log('\n\n\n\n');
  console.log('######## Trying to create a test user');
  newNymID = otapi.createNym(username);
  console.log('######## New test user = ' + newNymID);
  console.log('######## Trying to create an Account');
  newAccountID = otapi.createAccount(username + "'s hacker coins", newNymID, allAssetID[0]);
  console.log('######## New account = ' + newAccountID);
}

function removeAccountAndUser(){
  console.log('######## Trying to remove an Account');
  var accountDeleteSuccess = otapi.deleteAccount(newAccountID);
  console.log(((accountDeleteSuccess)?'######## Successfully removed the account':'######## Failed to remove the account'));
  console.log('######## Trying to remove the test user');
  var nymDeleteSuccess = otapi.deleteNym(newNymID);
  console.log(((nymDeleteSuccess)?'######## Successfully removed the nym':'######## Failed to remove the nym'));
}

//console.log(otapi.transferAssets('k5YaRDf1sJOpQzxmE2nTQJNczmi7GWtxwVmoBeKtlYO', 'aa2Tsh213OUm3fUnRb3W0DKLbU5owDaGZNH5nI6ZmQW', 100, 'test transfer'));

function createAndIssueNewAsset(newAssetName, tla, fraction){
  console.log('######## Trying to load an XML contract template');
  var xmldom = require('xmldom');
  var fs = require('fs');
  
  var DOMParser = xmldom.DOMParser;
  var DOMSerializer = xmldom.XMLSerializer;
  
  var fileData = fs.readFileSync('template.xml', 'ascii');
  var doc = new DOMParser().parseFromString(fileData.substring(0, fileData.length),'text/xml');
  
  
  console.log('######## Changing the name, tla, and fraction values in the contract = ' + newAssetName + ', ' + tla + ', ' + fraction);
  var currencyNode = doc.documentElement.getElementsByTagName('currency')[0];
  currencyNode.setAttribute('name', newAssetName);
  currencyNode.setAttribute('tla', tla);
  currencyNode.setAttribute('fraction', fraction);
  var assetContract = new DOMSerializer().serializeToString(doc);
    
  console.log('######## Trying to create the asset from the XML');
  var assetID = otapi.createNewAsset(otapi.mainUserID, assetContract);
  console.log('######## ASSET ID : ' + assetID);
  
  console.log('######## Retrieving the signed contract');
  var signedContract = otapi.getSignedAssetContract(assetID);
  console.log(signedContract);
  
  console.log('######## Issuing the signed contract');
  var issuerAccountID = otapi.issueAsset(otapi.mainUserID, assetID);
  console.log(issuerAccountID);
  
  
  
  doc = null;
  fileData = null;
  fs = null;
  xmldom = null;
  DOMParser = null;
  DOMSerializer = null;
}


function shutdown(){
  console.log(otapi.stopOTAPI());
}

function removeUserAndShutdown(){
  removeAccountAndUser();
  shutdown();
}




var allAssetID = [];
var newNymID, newAccountID;
var username = 'testuser10';
otapi.mainUserID = 'ya1AQQmaWnuntmDnoOjCmKpPXhmGuVAfkPwrgSc3nlf';
otapi.mainServerID = 'y0ca6JVtYSZuj1etoABAsaNJsU2Kb35AjeQZyZ0YCCF';

startup();
//var signedContract = otapi.getSignedAssetContract('oh4PYiMZKxbBX2CLz1cb5aCAjReCBll4xistOzfx6NC');
//console.log(otapi.issueAsset(otapi.mainUserID, 'oh4PYiMZKxbBX2CLz1cb5aCAjReCBll4xistOzfx6NC'));
//onsole.log(signedContract);
//listEverything();
//createAndIssueNewAsset('PURE Money', 'pre', 'mpre');

setTimeout(shutdown, 5000);
















//var http = require('http');
/*
http.createServer(function (req, res) {
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.end('Hello World\n');
}).listen(1337, '127.0.0.1');
**/
//console.log('Server running at http://127.0.0.1:1337/');



