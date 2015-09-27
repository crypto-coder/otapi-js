#include <string>
#include <ostream>
#include <iostream>
//#include <node.h>
#include <v8.h>
#include <inttypes.h>


#include <OT_ME.hpp>
#include <OTAPI.hpp>
#include <OpenTransactions.hpp>
#include <AssetContract.hpp>
#include <OTAsymmetricKey.hpp>
#include <OTBylaw.hpp>
#include <Identifier.hpp>
#include <Nym.hpp>
#include <OTParty.hpp>
#include <OTPassword.hpp>
#include <OTPasswordData.hpp>
#include <OTCaller.hpp>
#include <OTSmartContract.hpp>
#include <passwordCallback.hpp>

  using namespace v8;

  
  std::string mainNymID;
  std::string mainServerID;

 


static Handle<Value> GetNymID(Local<v8::String> property, const v8::AccessorInfo& info) {
  // Extract the C++ request object from the JavaScript wrapper.
  return v8::String::New(mainNymID.c_str());
}

static void SetNymID(Local<v8::String> property, Local<Value> value, const AccessorInfo& info) {
  v8::String::Utf8Value nymIDUTF(value);
  mainNymID = std::string(*nymIDUTF);
}

static Handle<Value> GetServerID(Local<v8::String> property, const v8::AccessorInfo& info) {
  // Extract the C++ request object from the JavaScript wrapper.
  return v8::String::New(mainServerID.c_str());
}

static void SetServerID(Local<v8::String> property, Local<Value> value, const AccessorInfo& info) {
  v8::String::Utf8Value serverIDUTF(value);
  mainServerID = std::string(*serverIDUTF);
}




//boolean success = otapi.startOTAPI('PASSPHRASE');
Handle<Value> StartOTAPI(const Arguments& args) {
  HandleScope scope;

  //Validate the supplied arguments
  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(v8::String::New("Wrong number of arguments. Expected a PASSPHRASE")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
      ThrowException(Exception::TypeError(v8::String::New("Expected a string-PASSPHRASE as the first parameter")));
    return scope.Close(False());
  }

  //Convert the supplied arguments
  v8::String::Utf8Value passphraseUTF(args[0]->ToString());
  std::string passphrase = std::string(*passphraseUTF);

  //Initialize the OTAPI library
  opentxs::OTAPI_Wrap::AppInit();

  //Set the password callback so we can login to the wallet
  static opentxs::OTCaller caller;
  static PasswordCallback passwordCallback(passphrase);
  caller.setCallback(&passwordCallback);
  OT_API_Set_PasswordCallback(caller);

  //Load the wallet
  bool otapiStarted = opentxs::OTAPI_Wrap::LoadWallet();

  if(otapiStarted){
    //Record the main server ID
    mainServerID = opentxs::OTAPI_Wrap::GetServer_ID(0);
  }

  return scope.Close(Boolean::New(otapiStarted));
}

//otapi.stopOTAPI();
Handle<Value> StopOTAPI(const Arguments& args){
  HandleScope scope;
  opentxs::OTAPI_Wrap::AppCleanup();
  return scope.Close(Null());
}

//string encodedText = otapi.encodeText('TEXTTOENCODE');
Handle<Value> EncodeText(const Arguments& args){
  HandleScope scope;

  //Make sure all the arguments have been supplied
  if (args.Length() == 0) {
    ThrowException(Exception::TypeError(v8::String::New("Wrong number of arguments. Expected: TEXTTOENCODE")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
      ThrowException(Exception::TypeError(v8::String::New("Expected a string-TEXTTOENCODE as the first parameter")));
    return scope.Close(False());
  }

  //Extract the values supplied as arguments
  v8::String::Utf8Value textToEncodeUTF(args[0]->ToString());
  std::string textToEncode = std::string(*textToEncodeUTF);

  //Encode the Text
  std::string encodedText = opentxs::OTAPI_Wrap::Encode(textToEncode, true);

  return scope.Close(v8::String::New(encodedText.c_str()));
}







//string nymID = otapi.createNym('NYM NAME');
//string nymID = otapi.createNym('NYM NAME', 'SERVER ID');
//string nymID = otapi.createNym('NYM NAME', 'SERVER ID', true_false_registerOnServer);
Handle<Value> CreateNym(const Arguments& args){
  HandleScope scope;

  if (args.Length() == 0) {
    ThrowException(Exception::TypeError(v8::String::New("Wrong number of arguments. Expected a NYM NAME")));
    return scope.Close(v8::String::New(""));
  }

  //Get the NYM NAME provided as an argument
  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(v8::String::New("Expected a string-NYM NAME as the first parameter")));
    return scope.Close(v8::String::New(""));
  }  
  v8::String::Utf8Value utf8StringNymName(args[0]->ToString());
  std::string nymName = std::string(*utf8StringNymName);

  std::string serverID = mainServerID;
  bool registerOnServer = true;
  if (args.Length() > 1) {
      //Get the SERVER ID provided as an argument
      if (!args[1]->IsString()) {
        ThrowException(Exception::TypeError(v8::String::New("Expected a string-SERVER ID as the second parameter")));
        return scope.Close(v8::String::New(""));
      }
      v8::String::Utf8Value utf8StringServerID(args[1]->ToString());
      serverID = std::string(*utf8StringServerID);

      if (args.Length() > 2) {
          //Get the REGISTER ON SERVER provided as an argument
          if (!args[2]->IsBoolean()) {
            ThrowException(Exception::TypeError(v8::String::New("Expected a boolean-REGISTER ON SERVER as the third parameter")));
            return scope.Close(v8::String::New(""));
          }
          registerOnServer = args[2]->ToBoolean()->Value();
      }
  }

  //Create the Nym ID and set the name to the one provided
  int32_t keySize = 1024;
  std::string nymID = OTAPI_Wrap::CreateNym(keySize, "", "");
  bool success = OTAPI_Wrap::SetNym_Name(nymID, nymID, nymName);

  //If there is no issue, the registerOnServer flag is set, register the new Nym with the server
  if(success){
    if(registerOnServer){
      int32_t requestNum = OTAPI_Wrap::registerNym(serverID, nymID);
      if(requestNum <= 0){
          //Error registering the Nym on the Server
          return scope.Close(v8::String::New(""));
      }
    }
    return scope.Close(v8::String::New(nymID.c_str()));
  }else{
    return scope.Close(v8::String::New(""));
  }
}


//boolean success = otapi.deleteNym('NYM ID');
//boolean success = otapi.deleteNym('NYM ID', 'SERVER ID');
Handle<Value> DeleteNym(const Arguments& args){
  HandleScope scope;

  if (args.Length() == 0) {
    ThrowException(Exception::TypeError(v8::String::New("Wrong number of arguments. Expected a string-NYM ID")));
    return scope.Close(False());
  }

  //Get the NYM ID provided as an argument
  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(v8::String::New("Expected a string-NYM ID as the first parameter")));
    return scope.Close(False());
  }
  v8::String::Utf8Value utf8StringNymID(args[0]->ToString());
  std::string nymID = std::string(*utf8StringNymID);
  Identifier providedNymID(nymID);
  Nym* providedNymIDPointer = OTAPI_Wrap::OTAPI()->GetNym(nymID, __FUNCTION__);
  if (nullptr == providedNymIDPointer){
      ThrowException(Exception::TypeError(v8::String::New("Could not load the Nym object from the provided NYM ID value.")));
      return scope.Close(False());
  }

  std::string serverID = mainServerID;
  if (args.Length() > 1) {
      //Get the SERVER ID provided as an argument
      if (!args[1]->IsString()) {
        ThrowException(Exception::TypeError(v8::String::New("Expected a string-SERVER ID as the second parameter")));
        return scope.Close(False());
      }
      v8::String::Utf8Value utf8StringServerID(args[1]->ToString());
      serverID = std::string(*utf8StringServerID);
  }

  //Check each Account in the Wallet to see if they can all be removed
  bool allNymAccountsCanBeRemoved = true;
  const int32_t accountCount = OTAPI_Wrap::OTAPI()->GetAccountCount();
  for (int32_t i = 0; i < accountCount; i++) {
      Identifier accountID;
      opentxs::String accountName;

      OTAPI_Wrap::OTAPI()->GetAccount(i, accountID, accountName);
      Account* account = OTAPI_Wrap::OTAPI()->GetAccount(accountID, __FUNCTION__);
      Identifier accountNymID(account->GetNymID());
      opentxs::String otStringAccountNymID(account->GetNymID());
      std::string accountNymIDString = otStringAccountNymID.Get();

      if (providedNymID == accountNymID) {
          //Check if we can delete this account
          if(!OTAPI_Wrap::Wallet_CanRemoveAccount(accountNymIDString)){
              allNymAccountsCanBeRemoved = false;
              break;
          }
      }
  }

  if(!allNymAccountsCanBeRemoved){
      ThrowException(Exception::TypeError(v8::String::New("At least one of the Accounts associated with this Nym still has unprocessed transactions or a non-zero balance and cannot be deleted.")));
      return scope.Close(False());
  }

  //Remove all Accounts associated with this Nym
  for (int32_t i = 0; i < accountCount; i++) {
      Identifier accountID;
      opentxs::String accountName;

      OTAPI_Wrap::OTAPI()->GetAccount(i, accountID, accountName);
      Account* account = OTAPI_Wrap::OTAPI()->GetAccount(accountID, __FUNCTION__);

      Identifier accountNymID(account->GetNymID());
      if (providedNymID == accountNymID) {
          //This account belongs to this Nym, so remove it
          int32_t requestStatus = OTAPI_Wrap::OTAPI()->deleteAssetAccount(account->GetRealNotaryID(), accountNymID, accountID);
          if(requestStatus <= 0){
              ThrowException(Exception::TypeError(v8::String::New("Failed to remove the Account from the Server for the Nym.")));
              return scope.Close(False());
          }
      }
  }

  //Check each Server in the Wallet to see if the Nym is registered there.  If so, unregister the Nym from the server
  const int32_t serverCount = OTAPI_Wrap::OTAPI()->GetServerCount();
  for (int32_t i = 0; i < serverCount; i++) {
      Identifier serverID;
      opentxs::String serverName;
      bool successGettingServer = OTAPI_Wrap::OTAPI()->GetServer(i, serverID, serverName);

      if (successGettingServer){
        if (!serverID.IsEmpty()) {
          const opentxs::String strNotaryID(serverID);
          std::string notaryIDString = strNotaryID.Get();

          if (providedNymIDPointer->IsRegisteredAtServer(strNotaryID)) {
              int32_t requestStatus = OTAPI_Wrap::unregisterNym(notaryIDString, nymID);
              if(requestStatus <= 0){
                  ThrowException(Exception::TypeError(v8::String::New("Failed to remove the Nym from the Server.")));
                  return scope.Close(False());
              }
          }
        }
      }
  }

  //Finally, remove the Nym from the wallet
  bool localRemovalSuccess = OTAPI_Wrap::Wallet_RemoveNym(nymID);
  if(localRemovalSuccess){
    return scope.Close(True());
  }else{
    ThrowException(Exception::TypeError(v8::String::New("Failed to remove the Nym from the local Wallet.")));
    return scope.Close(False());
  }
}







//string accountID = otapi.createAccount('ACCOUNT NAME', 'NYM ID', 'ASSET ID');
//string accountID = otapi.createAccount('ACCOUNT NAME', 'NYM ID', 'ASSET ID', 'SERVER ID');
Handle<Value> CreateAccount(const Arguments& args){
  HandleScope scope;

  if (args.Length() < 3) {
    ThrowException(Exception::TypeError(v8::String::New("Wrong number of arguments. Expected an ACCOUNT NAME, NYM ID, and ASSET ID")));
    return scope.Close(v8::String::New(""));
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(v8::String::New("Expected a string-ACCOUNT NAME for the first parameter")));
    return scope.Close(v8::String::New(""));
  }

  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(v8::String::New("Expected a string-NYM ID for the second parameter")));
    return scope.Close(v8::String::New(""));
  }

  if (!args[2]->IsString()) {
    ThrowException(Exception::TypeError(v8::String::New("Expected a string-ASSET ID for the third parameter")));
    return scope.Close(v8::String::New(""));
  }

  std::string serverID = mainServerID;
  if (args.Length() > 3) {
      //Get the SERVER ID provided as an argument
      if (!args[3]->IsString()) {
        ThrowException(Exception::TypeError(v8::String::New("Expected a string-SERVER ID as the fourth parameter")));
        return scope.Close(v8::String::New(""));
      }
      v8::String::Utf8Value utf8StringServerID(args[3]->ToString());
      serverID = std::string(*utf8StringServerID);
  }

  v8::String::Utf8Value utf8StringAccountName(args[0]->ToString());
  std::string accountName = std::string(*utf8StringAccountName);

  v8::String::Utf8Value utf8StringNymID(args[1]->ToString());
  std::string nymID = std::string(*utf8StringNymID);

  v8::String::Utf8Value utf8StringAssetID(args[2]->ToString());
  std::string assetID = std::string(*utf8StringAssetID);

  //Create the Asset Account using the default Server ID, Nym ID, and Asset ID
  OTAPI_Wrap::FlushMessageBuffer();
  OTAPI_Wrap::getRequestNumber(serverID, nymID);
  OT_ME madeEasy;
  std::string serverResponse = madeEasy.create_asset_acct(serverID, nymID, assetID);
  std::string accountID = OTAPI_Wrap::Message_GetNewAcctID(serverResponse);
  OTAPI_Wrap::SetAccountWallet_Name(accountID, nymID, accountName);

  OTAPI_Wrap::getRequestNumber(serverID, nymID);
  madeEasy.accept_inbox_items(accountID, 0, "");

  return scope.Close(v8::String::New(accountID.c_str()));
}


//boolean success = otapi.deleteAccount('ACCOUNT ID');
//boolean success = otapi.deleteAccount('ACCOUNT ID', 'SERVER ID');
Handle<Value> DeleteAccount(const Arguments& args){
  HandleScope scope;

  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(v8::String::New("Wrong number of arguments. Expected an ACCOUNT ID")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(v8::String::New("Expected a string-ACCOUNT ID for the first parameter")));
    return scope.Close(False());
  }

  std::string serverID = mainServerID;
  if (args.Length() > 1) {
      //Get the SERVER ID provided as an argument
      if (!args[1]->IsString()) {
        ThrowException(Exception::TypeError(v8::String::New("Expected a string-SERVER ID as the second parameter")));
        return scope.Close(False());
      }
      v8::String::Utf8Value utf8StringServerID(args[1]->ToString());
      serverID = std::string(*utf8StringServerID);
  }

  v8::String::Utf8Value utf8String(args[0]->ToString());
  std::string accountID = std::string(*utf8String);

  //Delete the Account from the server
  std::string nymID = OTAPI_Wrap::GetAccountWallet_NymID(accountID);
  OTAPI_Wrap::getRequestNumber(mainServerID, nymID);
  int32_t returnValue = OTAPI_Wrap::deleteAssetAccount(mainServerID, nymID, accountID);

  if(returnValue > 0){
    return scope.Close(True());
  }else{
    return scope.Close(False());
  }
}


//double accountBalance = otapi.getAccountBalance('ACCOUNT ID');
Handle<Value> GetAccountBalance(const Arguments& args){
  HandleScope scope;

  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(v8::String::New("Wrong number of arguments. Expected an ACCOUNT ID")));
    return scope.Close(Number::New(-1));
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(v8::String::New("Expected a string-ACCOUNT ID for the first parameter")));
    return scope.Close(Number::New(-1));
  }

  v8::String::Utf8Value utf8String(args[0]->ToString());
  std::string accountID = std::string(*utf8String);

  int64_t balance = OTAPI_Wrap::GetAccountWallet_Balance(accountID);
  return scope.Close(Number::New(balance));
}














void init(Handle<Object> exports) {
  exports->Set(v8::String::NewSymbol("startOTAPI"),
      FunctionTemplate::New(StartOTAPI)->GetFunction());
  exports->Set(v8::String::NewSymbol("stopOTAPI"),
      FunctionTemplate::New(StopOTAPI)->GetFunction());
  exports->Set(v8::String::NewSymbol("encodeText"),
      FunctionTemplate::New(EncodeText)->GetFunction());
  

  /*
  exports->Set(String::NewSymbol("getAccountIDList"),
      FunctionTemplate::New(GetAccountIDList)->GetFunction());
  exports->Set(String::NewSymbol("getNymIDList"),
      FunctionTemplate::New(GetNymIDList)->GetFunction());
  exports->Set(String::NewSymbol("getNymNameList"),
      FunctionTemplate::New(GetNymNameList)->GetFunction());
  exports->Set(String::NewSymbol("getAssetIDList"),
      FunctionTemplate::New(GetAssetIDList)->GetFunction());
  exports->Set(String::NewSymbol("getAssetNameList"),
      FunctionTemplate::New(GetAssetNameList)->GetFunction());
  */
  
  
  exports->Set(v8::String::NewSymbol("createNym"),
      FunctionTemplate::New(CreateNym)->GetFunction());
  exports->Set(v8::String::NewSymbol("deleteNym"),
      FunctionTemplate::New(DeleteNym)->GetFunction());  



  exports->Set(v8::String::NewSymbol("createAccount"),
      FunctionTemplate::New(CreateAccount)->GetFunction());
  exports->Set(v8::String::NewSymbol("deleteAccount"),
      FunctionTemplate::New(DeleteAccount)->GetFunction());
  exports->Set(v8::String::NewSymbol("getAccountBalance"),
      FunctionTemplate::New(GetAccountBalance)->GetFunction());





  /*
  exports->Set(String::NewSymbol("resyncNym"),
      FunctionTemplate::New(ResyncNym)->GetFunction());
  exports->Set(String::NewSymbol("getAccountComputedBalance"),
      FunctionTemplate::New(GetAccountComputedBalance)->GetFunction());
  
  
  
  
  
  exports->Set(String::NewSymbol("transferAssets"),
      FunctionTemplate::New(TransferAssets)->GetFunction());
  
  
  exports->Set(String::NewSymbol("signAssetContract"),
      FunctionTemplate::New(SignAssetContract)->GetFunction());
  exports->Set(String::NewSymbol("signAssetContractAndAddToWallet"),
      FunctionTemplate::New(SignAssetContractAndAddToWallet)->GetFunction());
  exports->Set(String::NewSymbol("getSignedAssetContract"),
      FunctionTemplate::New(GetSignedAssetContract)->GetFunction());
  exports->Set(String::NewSymbol("getAccountAssetContractID"),
      FunctionTemplate::New(GetAccountAssetContractID)->GetFunction());  
  exports->Set(String::NewSymbol("issueAsset"),
      FunctionTemplate::New(IssueAsset)->GetFunction());
  
  
  
  exports->Set(String::NewSymbol("createSmartContract"),
      FunctionTemplate::New(CreateSmartContract)->GetFunction());
  */
  
  
  exports->SetAccessor(v8::String::New("mainNymID"), GetNymID, SetNymID);
  exports->SetAccessor(v8::String::New("mainServerID"), GetServerID, SetServerID);

}

//NODE_MODULE(otapi, init)
