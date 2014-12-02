var otapi  = require('./build/Release/node_otapi');




function startup(){
  console.log(otapi.startOTAPI('ya1AQQmaWnuntmDnoOjCmKpPXhmGuVAfkPwrgSc3nlf', 'password'));
  console.log('issuer = ' + otapi.getAccountBalance('MtBJmjl6FXD51lnSAgENXwXh748BRe7j5TDd0vf8OXd'));
  console.log('comptroller = ' + otapi.getAccountBalance('oYeRsNAzcf0IcuqNEVtCdvW0fmt5F3FRucTJcygMcUd'));
  console.log(otapi.mainUserID);
  console.log(otapi.mainServerID);
}


function removeAccountAndUser(accountID, nymID){
  console.log('######## Trying to remove an Account');
  var accountDeleteSuccess = otapi.deleteAccount(accountID);
  console.log(((accountDeleteSuccess)?'######## Successfully removed the account':'######## Failed to remove the account'));
  removeUser(nymID);
}

function removeUser(nymID){
  console.log('######## Trying to remove the test user');
  var nymDeleteSuccess = otapi.deleteNym(nymID);
  console.log(((nymDeleteSuccess)?'######## Successfully removed the nym':'######## Failed to remove the nym'));
}


function shutdown(){
  console.log('issuer = ' + otapi.getAccountBalance('MtBJmjl6FXD51lnSAgENXwXh748BRe7j5TDd0vf8OXd'));
  console.log('comptroller = ' + otapi.getAccountBalance('oYeRsNAzcf0IcuqNEVtCdvW0fmt5F3FRucTJcygMcUd'));
  console.log(otapi.stopOTAPI());
}





var newNymID, newAccountID;
otapi.mainUserID = 'ya1AQQmaWnuntmDnoOjCmKpPXhmGuVAfkPwrgSc3nlf';
otapi.mainServerID = 'y0ca6JVtYSZuj1etoABAsaNJsU2Kb35AjeQZyZ0YCCF';

startup();
removeAccountAndUser('JYXBAqIqE8Cn9ZILGJ3sEcB9dz3gSQ6I2OutQervRCT', 'uTiiEmANuqniWquOLMaqbBXmhZZpSF1p0rhu7dOCGGD');  //testuser1
//removeUser('x030UWNvksu5yClSudExHyUbRz1rf6OGXWt5KyfaJKh'); //testuser2

shutdown();
