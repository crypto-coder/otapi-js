otapi-js
==========

Node.JS addon for interacting with Open Transactions using OTAPI.  This module provides the ability to load an Open Transactions wallet, create/modify accounts, create/modify new digital currencies, and transfer digital currencies between accounts.`



API
==========

boolean success = otapi.startOTAPI('PASSPHRASE');
otapi.stopOTAPI();

string encodedText = otapi.encodeText('TEXTTOENCODE');


string nymID = otapi.createNym('NYM NAME');
string nymID = otapi.createNym('NYM NAME', 'SERVER ID');
string nymID = otapi.createNym('NYM NAME', 'SERVER ID', true_false_registerOnServer);

boolean success = otapi.deleteNym('NYM ID');
boolean success = otapi.deleteNym('NYM ID', 'SERVER ID');

string accountID = otapi.createAccount('ACCOUNT NAME', 'NYM ID', 'ASSET ID');
string accountID = otapi.createAccount('ACCOUNT NAME', 'NYM ID', 'ASSET ID', 'SERVER ID');

boolean success = otapi.deleteAccount('ACCOUNT ID');
boolean success = otapi.deleteAccount('ACCOUNT ID', 'SERVER ID');

double accountBalance = otapi.getAccountBalance('ACCOUNT ID');
