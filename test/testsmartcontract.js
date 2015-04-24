var otapi = require("../index.js");


function startup(){
  console.log(otapi.startOTAPI('password'));
  //console.log(otapi.mainNymID);
  //console.log(otapi.mainServerID);
}


function shutdown(){
  console.log(otapi.stopOTAPI());
}






function createSmartContract(filePath, signingNymID){
  console.log('######## Trying to load an XML contract template');
  var xmldom = require('xmldom');
  var fs = require('fs');
  
  var DOMParser = xmldom.DOMParser;
  var DOMSerializer = xmldom.XMLSerializer;
  
  var fileData = fs.readFileSync(filePath, 'ascii');
  var doc = new DOMParser().parseFromString(fileData.substring(0, fileData.length),'text/xml');
  var smartContractNode = doc.documentElement.getElementsByTagName('smartContract')[0];
  //smartContractNode.setAttribute('serverID', 'testServerID');
  var smartContractXML = new DOMSerializer().serializeToString(doc);
    
  console.log('######## Trying to create a SmartContract from the XML');
  console.log('######## SIGNING NYM ID : ' + signingNymID);
  console.log('######## SMART CONTRACT XML : ' + smartContractXML);
  
  var smartContractID = otapi.createSmartContract(smartContractXML, signingNymID);
  console.log('######## SMART CONTRACT ID : ' + smartContractID);
  
  //console.log('######## Retrieving the signed contract');
  //var signedContract = otapi.getSignedAssetContract(assetID);
  //console.log(signedContract);
  
  //console.log('######## Issuing the signed contract');
  //var issuerAccountID = otapi.issueAsset(otapi.mainUserID, assetID);
  //console.log(issuerAccountID);
  
  
  
  doc = null;
  fileData = null;
  fs = null;
  xmldom = null;
  DOMParser = null;
  DOMSerializer = null;
}




function createTimeDecayCurrency(signingNymID, receiverNymID, assetTypeID, initialBalanceAmount, depositAccountID){
  try{
    var smartContractID = otapi.createSmartContract(signingNymID, receiverNymID, assetTypeID, initialBalanceAmount, depositAccountID);
    //console.log('######## SMART CONTRACT ID : ' + smartContractID);  
  }catch(e){
    console.log('######## ERROR : ' + e);  
  }
}



function resyncNym(nymID){
  otapi.resyncNym(nymID);  
}












var newNymID, newAccountID, HACKERCOIN_ASSET_ID, HACKERCOIN_ISSUER_ACCOUNT_ID, HACKERCOIN_COMPTROLLER_ACCOUNT_ID, RECEIVER_NYM_ID;
otapi.mainNymID = 'ya1AQQmaWnuntmDnoOjCmKpPXhmGuVAfkPwrgSc3nlf';
otapi.mainServerID = 'y0ca6JVtYSZuj1etoABAsaNJsU2Kb35AjeQZyZ0YCCF';
HACKERCOIN_ASSET_ID = 'dky86RGUPSAhdjcf6AyDtNtiFLvzfMdMa83b53WLXbI';
HACKERCOIN_ISSUER_ACCOUNT_ID = 'MtBJmjl6FXD51lnSAgENXwXh748BRe7j5TDd0vf8OXd';
HACKERCOIN_COMPTROLLER_ACCOUNT_ID = 'oYeRsNAzcf0IcuqNEVtCdvW0fmt5F3FRucTJcygMcUd';
RECEIVER_NYM_ID = 'STHNh73yLKwp2VYL6n3A7qoWJ5vMwjvRRWPIRaTauX5';


startup();

//var timeDecayCurrencySmartContract = createSmartContract('timeDecayCurrency.xml', otapi.mainNymID);

var currentBalance = otapi.getAccountBalance(HACKERCOIN_COMPTROLLER_ACCOUNT_ID);
console.log('TEST SMARTCONTRACT - HACKERCOIN COMPTROLLER ACCOUNT BALANCE = ' + currentBalance);

//var timeDecayCurrencySmartContract = createTimeDecayCurrency(otapi.mainNymID, RECEIVER_NYM_ID, HACKERCOIN_ASSET_ID, 100, HACKERCOIN_COMPTROLLER_ACCOUNT_ID);

currentBalance = otapi.getAccountBalance(HACKERCOIN_COMPTROLLER_ACCOUNT_ID);
console.log('TEST SMARTCONTRACT - HACKERCOIN COMPTROLLER ACCOUNT BALANCE = ' + currentBalance);

resyncNym(otapi.mainNymID);

currentBalance = otapi.getAccountBalance(HACKERCOIN_COMPTROLLER_ACCOUNT_ID);
console.log('TEST SMARTCONTRACT - HACKERCOIN COMPTROLLER ACCOUNT BALANCE = ' + currentBalance);

//removeUser('x030UWNvksu5yClSudExHyUbRz1rf6OGXWt5KyfaJKh'); //testuser2

shutdown();