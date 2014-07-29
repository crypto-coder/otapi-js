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
  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected a User ID and passphrase")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (User ID) is not in the correct format.")));
    return scope.Close(False());
  }

  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Second parameter (passphrase) is not in the correct format.")));
    return scope.Close(False());
  }

  //Convert the supplied arguments
  v8::String::Utf8Value userIDUTF(args[0]->ToString());
  mainUserID = std::string(*userIDUTF);

  v8::String::Utf8Value passphraseUTF(args[1]->ToString());
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


Handle<Value> GetAccountList(const Arguments& args){
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

    std::cout << "AccountType = " << OTAPI_Wrap::GetAccountWallet_Type(accountID) << std::endl;

    array->Set(i, String::New(accountID.c_str()));
  }

  // Return the value through Close.
  return scope.Close(array);
}




Handle<Value> TransferAssets(const Arguments& args){
  HandleScope scope;

  //Make sure all the arguments have been supplied
  if (args.Length() < 3) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected: From Account ID, To Account ID, Amount")));
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

  v8::String::Utf8Value toAccountUTF(args[1]->ToString());
  std::string toAccountID = std::string(*toAccountUTF);

  int64_t amount = args[2]->IntegerValue();

  std::string memo = "";
  if (args.Length() > 3) {
    v8::String::Utf8Value memoUTF(args[3]->ToString());
    memo = std::string(*memoUTF);
  }

  std::cout << "Transfer parameters = " << fromAccountID << ", " << toAccountID << ", " << amount << ", " << memo << std::endl;

  //Transfer the assets
  OTAPI_Wrap::notarizeTransfer(mainServerID, mainUserID, fromAccountID, toAccountID, amount, memo);

  //Process the inbox, approving all pending transcations
  //processAllTransfers();
  OT_ME madeEasy = OT_ME();
  madeEasy.accept_inbox_items(toAccountID, 0, "");
  madeEasy.accept_inbox_items(fromAccountID, 0, "");
  
  return scope.Close(True());
}


Handle<Value> IssueAssets(const Arguments& args){
  HandleScope scope;

  return scope.Close(Null());
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
  exports->Set(String::NewSymbol("getAccountBalance"),
      FunctionTemplate::New(GetAccountBalance)->GetFunction());
  exports->Set(String::NewSymbol("getAccountList"),
      FunctionTemplate::New(GetAccountList)->GetFunction());
  exports->Set(String::NewSymbol("transferAssets"),
      FunctionTemplate::New(TransferAssets)->GetFunction());
  exports->Set(String::NewSymbol("issueAssets"),
      FunctionTemplate::New(IssueAssets)->GetFunction());
  exports->SetAccessor(String::New("mainUserID"), GetUserID, SetUserID);
  exports->SetAccessor(String::New("mainServerID"), GetServerID, SetServerID);

}

NODE_MODULE(node_otapi, init)
