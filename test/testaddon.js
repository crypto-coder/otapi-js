// set up ========================
var otapi  = require('../build/Release/otapi');


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

function createNewAssetContractFromTemplate(newAssetName, tla, fraction){
  console.log('######## Trying to load an XML contract template');
  var xmldom = require('xmldom');
  var fs = require('fs');
  
  var DOMParser = xmldom.DOMParser;
  var DOMSerializer = xmldom.XMLSerializer;
  
  var fileData = fs.readFileSync('timeDecayCurrency-Sudden.xml', 'ascii');
  var doc = new DOMParser().parseFromString(fileData.substring(0, fileData.length),'text/xml');
    
  console.log('######## Changing the name, tla, and fraction values in the contract = ' + newAssetName + ', ' + tla + ', ' + fraction);
  var currencyNode = doc.documentElement.getElementsByTagName('currency')[0];
  currencyNode.setAttribute('name', newAssetName);
  currencyNode.setAttribute('tla', tla);
  currencyNode.setAttribute('fraction', fraction);
  var assetContract = new DOMSerializer().serializeToString(doc);
  
  console.log('######## Trying to create the asset from the XML');
  var assetID = otapi.createNewAsset(otapi.mainNymID, assetContract);
  console.log('######## ASSET ID : ' + assetID);
  
  return assetID;
}

function issueAssetContract(assetID){
  console.log('######## Retrieving the signed contract');
  var signedContract = otapi.getSignedAssetContract(assetID);
  //console.log(signedContract);
  
  console.log('######## Issuing the signed contract');
  var issuerAccountID = otapi.issueAsset(otapi.mainNymID, assetID);
  console.log(issuerAccountID);

}


function signAssetContractFromTemplate(newAssetName, tla, fraction, encodeClauses){
  console.log('######## Trying to load an XML contract template');
  var xmldom = require('xmldom');
  var fs = require('fs');
  
  var DOMParser = xmldom.DOMParser;
  var DOMSerializer = xmldom.XMLSerializer;
  
  var fileData = fs.readFileSync('timeDecayCurrency-Exponential.xml', 'ascii');
  var doc = new DOMParser().parseFromString(fileData.substring(0, fileData.length),'text/xml');
    
  console.log('######## Changing the name, tla, and fraction values in the contract = ' + newAssetName + ', ' + tla + ', ' + fraction);
  var currencyNode = doc.documentElement.getElementsByTagName('currency')[0];
  currencyNode.setAttribute('name', newAssetName);
  currencyNode.setAttribute('tla', tla);
  currencyNode.setAttribute('fraction', fraction);
    
  if(encodeClauses){
    console.log('######## Encoding all Clauses');
    var clauseNodes = doc.documentElement.getElementsByTagName('clause');
    var currentClauseNodeTextOriginal, currentClauseNodeTextEncoded = '';
    
    for(var i = 0; i < clauseNodes.length; i++){
      currentClauseNodeTextOriginal = clauseNodes[i].textContent;      
      currentClauseNodeTextEncoded = otapi.encodeText(currentClauseNodeTextOriginal);      
      var encodedTextLines = currentClauseNodeTextEncoded.split(/\n/g);
      var extractedEncodedTextLines = new Array();
      for(var j = 0; j < encodedTextLines.length; j++){	
	    if(j > 3 && j < (encodedTextLines.length - 3)){
	      extractedEncodedTextLines[extractedEncodedTextLines.length] = encodedTextLines[j];
	    }
      }      
      
      if(extractedEncodedTextLines.length == 1){
          clauseNodes[i].firstChild.data = extractedEncodedTextLines[0] + '\n';
      }else{
          clauseNodes[i].firstChild.data = extractedEncodedTextLines.join('\n');
      }
    }  
  }

  console.log('######## Trying to sign the new asset from the XML');
  var assetContract = new DOMSerializer().serializeToString(doc);
  var signedContract = otapi.signAssetContract(otapi.mainNymID, assetContract);
    
  return signedContract;
}

function issueSignedAssetContract(signedAssetContract){      
  console.log('######## Issuing the signed contract');
  var issuerAccountID = otapi.issueAsset(otapi.mainNymID, signedAssetContract);
  console.log(issuerAccountID);
  
  return issuerAccountID;
}


function shutdown(){
  console.log(otapi.stopOTAPI());
}

function removeUserAndShutdown(){
  removeAccountAndUser();
  shutdown();
}

// Issuer = k8Q6mYheE4PQydQGS9EzK2hiCFhCWm5Vz8ye6xFyk6S
// Comptroller = jWtf5ZbEtQ0AEZNwF89efSDwe5Oecv1otnIVgfxPRJG

var allAssetID = [];
var newNymID, newAccountID;
var username = 'testuser10';
otapi.mainNymID = 'ya1AQQmaWnuntmDnoOjCmKpPXhmGuVAfkPwrgSc3nlf';
otapi.mainServerID = 'y0ca6JVtYSZuj1etoABAsaNJsU2Kb35AjeQZyZ0YCCF';

startup();

var today = new Date(); 
var gapText = ' - ';
var currencySuffix = gapText.concat(today.getFullYear(), today.getMonth()+1, today.getDate(), today.getHours(), today.getMinutes(), today.getSeconds());

//var timeDecayCurrencyAssetID = createNewAssetContractFromTemplate('time Decay Currency' + currencySuffix, 'TDC', 'mTDC');
//issueAssetContract(timeDecayCurrencyAssetID);

//console.log('######## SIGNING AN XML ASSET CONTRACT');
//var signedTimeDecayCurrencyContract = signAssetContractFromTemplate('time Decay Currency' + currencySuffix, 'TDC', 'mTDC', true);

//console.log('######## ISSUING A SIGNED ASSET CONTRACT');
//var issuerAccountID = issueSignedAssetContract(signedTimeDecayCurrencyContract);
//console.log(issuerAccountID);



//console.log('######## GETTING THE ASSET CONTRACT ID');
//var assetID = otapi.getAccountAssetContractID(issuerAccountID);
//console.log(assetID);

//console.log('######## CREATING A NEW ACCOUNT WITH THE ASSET CONTRACT');
//var comptrollerAccountID = otapi.createAccount('Comptroller', otapi.mainNymID, assetID);
//console.log(comptrollerAccountID);

//console.log('######## TRANSFERRING ASSETS BETWEEN ASSET ACCOUNTS');
//otapi.transferAssets(issuerAccountID, comptrollerAccountID, 10000);

var comptrollerAccountID = 'bBDxLcSosfIRI7I1nqx7RmK29CdZcX1RruPe7Y9HM0R';

console.log('######## GETTING THE COMPUTED BALANCE FOR THE DESTINATION ACCOUNT');
var computedBalance = otapi.getAccountComputedBalance(comptrollerAccountID);
console.log(computedBalance);

//listEverything();
setTimeout(shutdown, 5000);

