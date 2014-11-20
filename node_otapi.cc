#include <string>
#include <ostream>
#include <iostream>
#include <node.h>
#include <v8.h>

#include <OT_ME.hpp>
#include <OTAPI.hpp>
#include <OTAsymmetricKey.hpp>
#include <OTIdentifier.hpp>
#include <OTPassword.hpp>
#include <OTCaller.hpp>
#include <passwordCallback.hpp>


using namespace v8;

std::string mainUserID = "";
std::string mainServerID = "";


static Handle<Value> GetUserID(Local<String> property, const v8::AccessorInfo& info) {
  // Extract the C++ request object from the JavaScript wrapper.
  return v8::String::New(mainUserID.c_str());
}

static void SetUserID(Local<String> property, Local<Value> value, const AccessorInfo& info) {
  v8::String::Utf8Value userIDUTF(value);
  mainUserID = std::string(*userIDUTF);
}

static Handle<Value> GetServerID(Local<String> property, const v8::AccessorInfo& info) {
  // Extract the C++ request object from the JavaScript wrapper.
  return v8::String::New(mainServerID.c_str());
}

static void SetServerID(Local<String> property, Local<Value> value, const AccessorInfo& info) {
  v8::String::Utf8Value serverIDUTF(value);
  mainServerID = std::string(*serverIDUTF);
}




Handle<Value> StartOTAPI(const Arguments& args) {
  HandleScope scope;

  //Validate the supplied arguments
  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected a Passphrase")));
    return scope.Close(False());
  }
  
  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (passphrase) is not in the correct format.")));
    return scope.Close(False());
  }

  //Convert the supplied arguments
  v8::String::Utf8Value passphraseUTF(args[0]->ToString());
  std::string passphrase = std::string(*passphraseUTF);

  //Initialize the OTAPI library
  OTAPI_Wrap::AppInit();

  //Set the password callback so we can login to the wallet
  static OTCaller caller;
  static PasswordCallback passwordCallback(passphrase);
  caller.setCallback(&passwordCallback);
  OT_API_Set_PasswordCallback(caller);
  
  //Load the wallet
  bool otapiStarted = OTAPI_Wrap::LoadWallet();

  if(otapiStarted){
    //Record the main server ID
    mainServerID = OTAPI_Wrap::GetServer_ID(0);
  }

  return scope.Close(Boolean::New(otapiStarted));
}



Handle<Value> StopOTAPI(const Arguments& args){
  HandleScope scope;
  OTAPI_Wrap::AppCleanup();
  return scope.Close(Null());
}





Handle<Value> CreateNym(const Arguments& args){
  HandleScope scope;

  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected a Name")));
    return scope.Close(String::New(""));
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected a Name")));
    return scope.Close(String::New(""));
  }

  v8::String::Utf8Value utf8String(args[0]->ToString());
  std::string nymName = std::string(*utf8String);
  
  //Create the Nym ID and set the name to the one provided
  int32_t keySize = 1024;
  std::string nymID = OTAPI_Wrap::CreateNym(keySize, "", "");
  bool success = OTAPI_Wrap::SetNym_Name(nymID, nymID, nymName);
  
  //If there is no issue, register the new Nym with the server
  if(success){    
    OTAPI_Wrap::getRequest(mainServerID, nymID);
    OTAPI_Wrap::createUserAccount(mainServerID, nymID);
    return scope.Close(v8::String::New(nymID.c_str()));
  }else{
    return scope.Close(String::New(""));
  }
}

Handle<Value> DeleteNym(const Arguments& args){
  HandleScope scope;

  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected a Nym ID")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected a Nym ID")));
    return scope.Close(False());
  }

  v8::String::Utf8Value utf8String(args[0]->ToString());
  std::string nymID = std::string(*utf8String);
  
  //Delete the Nym from the server then remove it locally
  OTAPI_Wrap::getRequest(mainServerID, nymID);
  int32_t removeRemovalCode = OTAPI_Wrap::deleteUserAccount(mainServerID, nymID);  
  if(removeRemovalCode > 0){
    bool localRemovalSuccess = OTAPI_Wrap::Wallet_RemoveNym(nymID);
    if(localRemovalSuccess){
      return scope.Close(True());
    }else{
      return scope.Close(False());
    }
  }else{
    return scope.Close(False());
  }
}




Handle<Value> CreateAccount(const Arguments& args){
  HandleScope scope;

  if (args.Length() < 3) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected an Account Name, Nym ID, and Asset ID")));
    return scope.Close(String::New(""));
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected an Account Name for the first parameter")));
    return scope.Close(String::New(""));
  }
  
  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected a Nym ID for the second parameter")));
    return scope.Close(String::New(""));
  }

  if (!args[2]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected a Asset ID for the third parameter")));
    return scope.Close(String::New(""));
  }

  v8::String::Utf8Value utf8StringAccountName(args[0]->ToString());
  std::string accountName = std::string(*utf8StringAccountName);
  
  v8::String::Utf8Value utf8StringNymID(args[1]->ToString());
  std::string nymID = std::string(*utf8StringNymID);
  
  v8::String::Utf8Value utf8StringAssetID(args[2]->ToString());
  std::string assetID = std::string(*utf8StringAssetID);
  
  //Create the Asset Account using the default Server ID, Nym ID, and Asset ID
  OTAPI_Wrap::getRequest(mainServerID, nymID);
  OT_ME madeEasy = OT_ME();
  std::string serverResponse = madeEasy.create_asset_acct(mainServerID, nymID, assetID);
  std::string accountID = OTAPI_Wrap::Message_GetNewAcctID(serverResponse);
  OTAPI_Wrap::SetAccountWallet_Name(accountID, nymID, accountName);
  
  OTAPI_Wrap::getRequest(mainServerID, nymID);
  madeEasy.accept_inbox_items(accountID, 0, "");
  
  return scope.Close(v8::String::New(accountID.c_str()));
}

Handle<Value> GetAccountBalance(const Arguments& args){
  HandleScope scope;

  OTIdentifier nym_id("IirfVWfx09PmHAznlJUR93jvFZmSZ6Wh5fYHkNERxKj");
  OTIdentifier server_id("xyzl2I9VFVJP7ujXqUfdyf4Eoj6tQl2sOKnXcs741TH");
  OTIdentifier account_id("k5YaRDf1sJOpQzxmE2nTQJNczmi7GWtxwVmoBeKtlYO");

  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected an Account ID")));
    return scope.Close(Number::New(-1));
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected an Account ID")));
    return scope.Close(Number::New(-1));
  }

  v8::String::Utf8Value utf8String(args[0]->ToString());
  std::string accountID = std::string(*utf8String);

  int64_t balance = OTAPI_Wrap::GetAccountWallet_Balance(accountID);
  return scope.Close(Number::New(balance));
}

Handle<Value> DeleteAccount(const Arguments& args){
  HandleScope scope;

  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected an Account ID")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected a Account ID")));
    return scope.Close(False());
  }

  v8::String::Utf8Value utf8String(args[0]->ToString());
  std::string accountID = std::string(*utf8String);
  
  //Delete the Account from the server
  std::string nymID = OTAPI_Wrap::GetAccountWallet_NymID(accountID);
  OTAPI_Wrap::getRequest(mainServerID, nymID);
  int32_t returnValue = OTAPI_Wrap::deleteAssetAccount(mainServerID, nymID, accountID);  
  
  if(returnValue > 0){
    return scope.Close(True());
  }else{
    return scope.Close(False());
  }
}







Handle<Value> GetAccountIDList(const Arguments& args){
  // We will be creating temporary handles so we use a handle scope.
  HandleScope scope;

  //Determine how many accounts are present
  int32_t accountCount = OTAPI_Wrap::GetAccountCount();

  // Create a new empty array.
  Handle<Array> array = Array::New(accountCount);

  // Return an empty result if there was an error creating the array.
  if (array.IsEmpty())
    return Handle<Array>();

  // Add all the account names
  for(int32_t i = 0; i < accountCount; i++){
    std::string accountID = OTAPI_Wrap::GetAccountWallet_ID(i);

    //std::cout << "AccountType = " << OTAPI_Wrap::GetAccountWallet_Type(accountID) << std::endl;

    array->Set(i, String::New(accountID.c_str()));
  }

  // Return the value through Close.
  return scope.Close(array);
}


Handle<Value> GetNymIDList(const Arguments& args){
  // We will be creating temporary handles so we use a handle scope.
  HandleScope scope;

  //Determine how many nyms are present
  int32_t nymCount = OTAPI_Wrap::GetNymCount();

  // Create a new empty array.
  Handle<Array> array = Array::New(nymCount);

  // Return an empty result if there was an error creating the array.
  if (array.IsEmpty())
    return Handle<Array>();

  // Add all the nyms
  for(int32_t i = 0; i < nymCount; i++){
    std::string nymID = OTAPI_Wrap::GetNym_ID(i);
    array->Set(i, String::New(nymID.c_str()));
  }

  // Return the value through Close.
  return scope.Close(array);
}
Handle<Value> GetNymNameList(const Arguments& args){
  // We will be creating temporary handles so we use a handle scope.
  HandleScope scope;

  //Determine how many nyms are present
  int32_t nymCount = OTAPI_Wrap::GetNymCount();

  // Create a new empty array.
  Handle<Array> array = Array::New(nymCount);

  // Return an empty result if there was an error creating the array.
  if (array.IsEmpty())
    return Handle<Array>();

  // Add all the nyms
  for(int32_t i = 0; i < nymCount; i++){
    std::string nymID = OTAPI_Wrap::GetNym_ID(i);
    std::string nymName = OTAPI_Wrap::GetNym_Name(nymID);
    array->Set(i, String::New(nymName.c_str()));
  }

  // Return the value through Close.
  return scope.Close(array);
}


Handle<Value> GetAssetIDList(const Arguments& args){
  // We will be creating temporary handles so we use a handle scope.
  HandleScope scope;

  //Determine how many assets are present
  int32_t assetCount = OTAPI_Wrap::GetAssetTypeCount();

  // Create a new empty array.
  Handle<Array> array = Array::New(assetCount);

  // Return an empty result if there was an error creating the array.
  if (array.IsEmpty())
    return Handle<Array>();

  // Add all the asset names
  for(int32_t i = 0; i < assetCount; i++){
    std::string assetID = OTAPI_Wrap::GetAssetType_ID(i);

    array->Set(i, String::New(assetID.c_str()));
  }

  // Return the value through Close.
  return scope.Close(array);
}
Handle<Value> GetAssetNameList(const Arguments& args){
  // We will be creating temporary handles so we use a handle scope.
  HandleScope scope;

  //Determine how many assets are present
  int32_t assetCount = OTAPI_Wrap::GetAssetTypeCount();

  // Create a new empty array.
  Handle<Array> array = Array::New(assetCount);

  // Return an empty result if there was an error creating the array.
  if (array.IsEmpty())
    return Handle<Array>();

  // Add all the asset names
  for(int32_t i = 0; i < assetCount; i++){
    std::string assetID = OTAPI_Wrap::GetAssetType_ID(i);
    std::string assetName = OTAPI_Wrap::GetAssetType_Name(assetID);

    array->Set(i, String::New(assetName.c_str()));
  }

  // Return the value through Close.
  return scope.Close(array);
}








Handle<Value> TransferAssets(const Arguments& args){
  HandleScope scope;

  //Make sure all the arguments have been supplied
  if (args.Length() < 3) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected: From Account ID, To Account ID, Amount, Memo")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (From Account ID) is not in the correct format.")));
    return scope.Close(False());
  }

  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Second parameter (To Account ID) is not in the correct format.")));
    return scope.Close(False());
  }

  if (!args[2]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Third parameter (Amount) is not a number.")));
    return scope.Close(False());
  }
  
  //Extract the values supplied as arguments
  v8::String::Utf8Value fromAccountUTF(args[0]->ToString());
  std::string fromAccountID = std::string(*fromAccountUTF);
  
  std::string fromNymID = OTAPI_Wrap::GetAccountWallet_NymID(fromAccountID);  

  v8::String::Utf8Value toAccountUTF(args[1]->ToString());
  std::string toAccountID = std::string(*toAccountUTF);

  int64_t amount = args[2]->IntegerValue();

  std::string memo = "";
  if (args.Length() > 3) {
    v8::String::Utf8Value memoUTF(args[3]->ToString());
    memo = std::string(*memoUTF);
  }

  std::cout << "Transfer parameters = " << fromNymID << ", " << fromAccountID << ", " << toAccountID << ", " << amount << ", " << memo << std::endl;

  //Transfer the assets
  OTAPI_Wrap::notarizeTransfer(mainServerID, fromNymID, fromAccountID, toAccountID, amount, memo);

  //Process the inbox, approving all pending transcations
  //processAllTransfers();
  OT_ME madeEasy = OT_ME();
  madeEasy.accept_inbox_items(toAccountID, 0, "");
  madeEasy.accept_inbox_items(fromAccountID, 0, "");
  
  return scope.Close(True());
}





Handle<Value> CreateNewAsset(const Arguments& args){
  HandleScope scope;

  //Make sure all the arguments have been supplied
  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected: Nym ID, Asset Contract XML")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (Nym ID) is not in the correct format.")));
    return scope.Close(False());
  }

  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Second parameter (Asset Contract XML) is not in the correct format.")));
    return scope.Close(False());
  }
  
  //Extract the values supplied as arguments
  v8::String::Utf8Value nymIDUTF(args[0]->ToString());
  std::string nymID = std::string(*nymIDUTF);
  
  v8::String::Utf8Value assetContractXMLUTF(args[1]->ToString());
  std::string assetContractXML = std::string(*assetContractXMLUTF);


  std::cout << "Create Asset parameters = " << nymID << ", " << assetContractXML << std::endl;
  
  //Create the new asset
  std::string assetID = OTAPI_Wrap::CreateAssetContract(nymID, assetContractXML);
  
  
  //std::string OTAPI_Wrap::GetAssetType_Contract(const std::string& ASSET_TYPE_ID)
  //int32_t OTAPI_Wrap::AddAssetContract(const std::string& strContract)
  //std::string OTAPI_Wrap::SignContract(const std::string& SIGNER_NYM_ID, const std::string& THE_CONTRACT)
  //std::string OTAPI_Wrap::LoadAssetContract(const std::string& ASSET_TYPE_ID)
  
  return scope.Close(v8::String::New(assetID.c_str()));
}

Handle<Value> GetSignedAssetContract(const Arguments& args){
  HandleScope scope;

  //Make sure all the arguments have been supplied
  if (args.Length() == 0) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected: Asset ID")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (Asset ID) is not in the correct format.")));
    return scope.Close(False());
  }
  
  //Extract the values supplied as arguments
  v8::String::Utf8Value assetIDUTF(args[0]->ToString());
  std::string assetID = std::string(*assetIDUTF);
  
  //Retrieve the signed asset contract
  std::string signedContract = OTAPI_Wrap::GetAssetType_Contract(assetID);
  
  return scope.Close(v8::String::New(signedContract.c_str()));
}

Handle<Value> IssueAsset(const Arguments& args){
  HandleScope scope;

  //Make sure all the arguments have been supplied
  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected: Nym ID, Asset ID (or the Signed Asset Contract)")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (Nym ID) is not in the correct format.")));
    return scope.Close(False());
  }
  
  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Second parameter (Asset ID (or Signed Asset Contract)) is not in the correct format.")));
    return scope.Close(False());
  }
  
  
  //Extract the values supplied as arguments
  v8::String::Utf8Value nymIDUTF(args[0]->ToString());
  std::string nymID = std::string(*nymIDUTF);
  
  v8::String::Utf8Value assetContractUTF(args[1]->ToString());
  std::string assetContract = std::string(*assetContractUTF);
  if(assetContract.length() < 50){
    //They probably gave us an Asset ID.  Retrieve the signed contract
    assetContract = OTAPI_Wrap::GetAssetType_Contract(assetContract);
  }
  
  std::string serverID = mainServerID;
  if (args.Length() > 2) {
    v8::String::Utf8Value serverIDUTF(args[2]->ToString());
    serverID = std::string(*serverIDUTF);
  }
  
  //std::cout << "Issue Asset parameters = " << nymID << ", " << serverID << ", " << assetContract << std::endl;
  
  
  //Issue the Asset on the Server for this Nym
  OT_ME madeEasy = OT_ME();
  std::string serverResponse = madeEasy.issue_asset_type(serverID, nymID, assetContract);
  //std::cout << "Server Response = " << serverResponse << std::endl;
  
  std::string issuerAccountID = OTAPI_Wrap::Message_GetNewIssuerAcctID(serverResponse);
  std::cout << "Issuer Account ID = " << issuerAccountID << std::endl;
  
  OTAPI_Wrap::SetAccountWallet_Name(issuerAccountID, nymID, "Issuer Account");
  
  OTAPI_Wrap::getRequest(serverID, nymID);
  madeEasy.accept_inbox_items(issuerAccountID, 0, "");
    
  return scope.Close(v8::String::New(issuerAccountID.c_str()));
}







void processAllTransfers(){
  
/*
if (!OT_ME_insure_enough_nums(10, strServerID, strMyNymID))
    {
        return (-1)
    }
    // -------------------------------------------------------
    var strInbox = OT_API_LoadInbox(strServerID, strMyNymID, strMyAcctID) // Returns NULL, or an inbox.

        if (!VerifyStringVal(strInbox))
        {
                OT_API_Output(0, "\n\n OT_API_LoadInbox: Failed.\n\n")
        return -1
        }
        else    // Success!
        {
                OT_API_Output(0, "\n\n") // stderr
//              print(strInbox)          // stdout

        var nCount = OT_API_Ledger_GetCount(strServerID, strMyNymID, strMyAcctID, strInbox)

        if (VerifyIntVal(nCount) && (nCount > 0))
        {
            // NOTE!!! DO **NOT** create the response ledger until the FIRST iteration of the below loop that actually
            // creates a transaction response! If that "first iteration" never comes (due to receipts being skipped, etc)
            // then OT_API_Transaction_CreateResponse will never get called, and therefore Ledger_CreateResponse should
            // also not be called, either. (Nor should OT_API_Ledger_FinalizeResponse, etc.)
            //
            var strResponseLEDGER = ""
            // ----------------------------------------------
            var nIndicesCount = VerifyStringVal(strIndices) ? OT_API_NumList_Count(strIndices) : 0
            // ------------------------------------------------------------------------
            for (var nIndex = 0; nIndex < nCount; ++nIndex)
            {
                var bContinue = false  // I'm hacking my own "continue" since the language doesn't support it....

                var strTrans = OT_API_Ledger_GetTransactionByIndex(strServerID, strMyNymID, strMyAcctID, strInbox, nIndex)
                // ----------------------------------------------------------
                // nItemType  == 0 for all, 1 for transfers only, 2 for receipts only.
                // strIndices == "" for "all indices"
                //
                if (nItemType > 0) // 0 means "all", so we don't have to skip anything based on type, if it's 0.
                {
                    var strTransType = OT_API_Transaction_GetType(strServerID, strMyNymID, strMyAcctID, strTrans)

                    // incoming transfer
                    if (("pending" == strTransType) && (1 != nItemType))
                    {
                        // if it IS an incoming transfer, but we're doing receipts, then skip it.
//                      continue // language doesn't support this
                        bContinue = true
                    }
                    // receipt
                    else if (!bContinue && ("pending" != strTransType) && (2 != nItemType))
                    {
                        // if it is NOT an incoming transfer, then it's a receipt. If we're not doing receipts, then skip it.
//                      continue // language doesn't support this
                        bContinue = true
                    }
                }
                // -------------------------
                if (!bContinue)
                {
                    // ----------------------------------------------------------
                    // - If NO indices are specified, process them ALL.
                    //
                    // - If indices are specified, but the current index is not on
                    //   that list, then continue...
                    //
                    if ((nIndicesCount > 0) && !OT_API_NumList_VerifyQuery(strIndices, nIndex.to_string()))
                    {
//                      continue  // language doesn't support continue.
                        bContinue = true
                    }
                    // ----------------------------------------------------------
                    else if (!bContinue)
                    {
                        // By this point we know we actually have to call OT_API_Transaction_CreateResponse
                        // Therefore, if OT_API_Ledger_CreateResponse has not yet been called (which it won't
                        // have been, the first time we hit this in this loop), then we call it here this one
                        // time, to get things started...
                        //
                        if (!VerifyStringVal(strResponseLEDGER))
                        {
                            strResponseLEDGER = OT_API_Ledger_CreateResponse(strServerID, strMyNymID, strMyAcctID, strInbox)

                            if (!VerifyStringVal(strResponseLEDGER))
                            {
                                OT_API_Output(0, "\n\nFailure: OT_API_Ledger_CreateResponse returned NULL.\n")
                                return (-1)
                            }
                        }
                        // ----------------------------
                        // By this point, we know the ledger response exists, and we know we have to create
                        // a transaction response to go inside of it, so let's do that next...
                        //
                        var strNEW_ResponseLEDGER = OT_API_Transaction_CreateResponse(strServerID, strMyNymID, strMyAcctID, strResponseLEDGER, strTrans, true) // accept = true (versus rejecting a pending transfer, for example.)

                        if (!VerifyStringVal(strNEW_ResponseLEDGER))
                        {
                            OT_API_Output(0, "\n\nFailure: OT_API_Transaction_CreateResponse returned NULL.\n")
                            return (-1)
                        }
                        strResponseLEDGER = strNEW_ResponseLEDGER
                    }
                }
            } // for
            // ----------------------------------
            if (!VerifyStringVal(strResponseLEDGER))
            {
                // This means there were receipts in the box, but they were skipped.
                // And after the skipping was done, there were no receipts left.
                // So we can't just say "the box is empty" because it's not. But nevertheless,
                // we aren't actually processing any of them, so we return 0 AS IF the box
                // had been empty. (Because this is not an error condition. Just a "no op".)
                //
                return 0
            }
            // -------------------------------------------
            // Below this point, we know strResponseLEDGER needs to be sent,
            // so let's finalize it.
            //
            var strFinalizedResponse = OT_API_Ledger_FinalizeResponse(strServerID, strMyNymID, strMyAcctID, strResponseLEDGER)

            if (!VerifyStringVal(strFinalizedResponse))
            {
                OT_API_Output(0, "\n\nFailure: OT_API_Ledger_FinalizeResponse returned NULL.\n")
                return (-1)
            }
            // ***************************************************************

            // Server communications are handled here...
            //
            var strResponse = OT_ME_process_inbox(strServerID, strMyNymID, strMyAcctID, strFinalizedResponse)
            var strAttempt  = "process_inbox"

            // ***************************************************************


            var nInterpretReply = InterpretTransactionMsgReply(strServerID, strMyNymID, strMyAcctID, strAttempt, strResponse)

            if (1 == nInterpretReply)
            {
                // Download all the intermediary files (account balance, inbox, outbox, etc)
                // since they have probably changed from this operation.
                //
                var bRetrieved = OT_ME_retrieve_account(strServerID, strMyNymID, strMyAcctID, true) //bForceDownload defaults to false.

                OT_API_Output(0, "\n\nServer response ("+strAttempt+"): SUCCESS processing/accepting inbox.\n")
                OT_API_Output(0, (bRetrieved ? "Success" : "Failed") + " retrieving intermediary files for account.\n")
            }

            // ***************************************************************
            //
            // Success!
            //
            return nInterpretReply

        } // VerifyIntVal (nCount) && nCount > 0

        OT_API_Output(0, "The asset account inbox is empty.\n\n")
    }

    0
*/

}




void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("startOTAPI"),
      FunctionTemplate::New(StartOTAPI)->GetFunction());
  exports->Set(String::NewSymbol("stopOTAPI"),
      FunctionTemplate::New(StopOTAPI)->GetFunction());
  
  
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
  
  
  
  exports->Set(String::NewSymbol("createNym"),
      FunctionTemplate::New(CreateNym)->GetFunction());
  exports->Set(String::NewSymbol("deleteNym"),
      FunctionTemplate::New(DeleteNym)->GetFunction());
  exports->Set(String::NewSymbol("createAccount"),
      FunctionTemplate::New(CreateAccount)->GetFunction()); 
  exports->Set(String::NewSymbol("deleteAccount"),
      FunctionTemplate::New(DeleteAccount)->GetFunction());   
  exports->Set(String::NewSymbol("getAccountBalance"),
      FunctionTemplate::New(GetAccountBalance)->GetFunction());
  
  
  
  
  
  exports->Set(String::NewSymbol("transferAssets"),
      FunctionTemplate::New(TransferAssets)->GetFunction());
  
  
  exports->Set(String::NewSymbol("createNewAsset"),
      FunctionTemplate::New(CreateNewAsset)->GetFunction());
  exports->Set(String::NewSymbol("issueAsset"),
      FunctionTemplate::New(IssueAsset)->GetFunction());
  exports->Set(String::NewSymbol("getSignedAssetContract"),
      FunctionTemplate::New(GetSignedAssetContract)->GetFunction());
  
  
  exports->SetAccessor(String::New("mainUserID"), GetUserID, SetUserID);
  exports->SetAccessor(String::New("mainServerID"), GetServerID, SetServerID);

}

NODE_MODULE(node_otapi, init)
