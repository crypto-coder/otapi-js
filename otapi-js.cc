#include <string>
#include <ostream>
#include <iostream>
#include <node.h>
#include <v8.h>

#include <OT_ME.hpp>
#include <OTAPI.hpp>
#include <OpenTransactions.hpp>
#include <OTAssetContract.hpp>
#include <OTAsymmetricKey.hpp>
#include <OTBylaw.hpp>
#include <OTIdentifier.hpp>
#include <OTPseudonym.hpp>
#include <OTParty.hpp>
#include <OTPassword.hpp>
#include <OTPasswordData.hpp>
#include <OTCaller.hpp>
#include <OTSmartContract.hpp>
#include <passwordCallback.hpp>


  using namespace v8;

  
  std::string mainNymID;
  std::string mainServerID;
  
  
//   v8::Persistent<v8::Function> constructor;
//   otapi_js::otapi_js() : mainServerID(""), mainNymID("") {}
//   otapi_js::otapi_js(std::string & serverID, std::string & userID) : mainServerID(serverID), mainNymID(userID) { }
//   otapi_js::~otapi_js() {}
//   void otapi_js::Init(Handle<Object> exports) {
//     Isolate* isolate = Isolate::GetCurrent();
// 
//     // Prepare constructor template
//     Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
//     tpl->SetClassName(String::NewFromUtf8(isolate, "otapi_js"));
//     tpl->InstanceTemplate()->SetInternalFieldCount(2);
// 
//     // Prototype
//     NODE_SET_PROTOTYPE_METHOD(tpl, "plusOne", PlusOne);
// 
//     constructor.Reset(isolate, tpl->GetFunction());
//     exports->Set(String::NewFromUtf8(isolate, "otapi_js"),
// 		tpl->GetFunction());
//   }
// 
//   void otapi_js::New(const FunctionCallbackInfo<Value>& args) {
//     Isolate* isolate = Isolate::GetCurrent();
//     HandleScope scope(isolate);
// 
//     if (args.IsConstructCall()) {
//       // Invoked as constructor: `new otapi_js(...)`
// 	
//       if(args.Length() == 1 || args.Length() > 2) {
// 	ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected : MainServerID, MainNymID")));
// 	return scope.Close(String::New(""));
//       }
// 
//       if (!args[0]->IsString()) {
// 	ThrowException(Exception::TypeError(String::New("Expected the MainServerID for the first parameter")));
// 	return scope.Close(String::New(""));
//       }
//       
//       if (!args[1]->IsString()) {
// 	ThrowException(Exception::TypeError(String::New("Expected the MainNymID for the second parameter")));
// 	return scope.Close(String::New(""));
//       }
// 
//       String::Utf8Value utf8StringMainServerID(args[0]->ToString());
//       String::Utf8Value utf8StringMainNymID(args[1]->ToString());
// 	  
//       otapi_js* obj = new otapi_js(std::string(*utf8StringMainServerID), std::string(*utf8StringMainNymID));
//       obj->Wrap(args.This());
//       args.GetReturnValue().Set(args.This());
//     } else {
//       // Invoked as plain function `otapi_js(...)`, turn into construct call.
//       const int argc = 1;
//       Local<Value> argv[argc] = { args[0] };
//       Local<Function> cons = Local<Function>::New(isolate, constructor);
//       args.GetReturnValue().Set(cons->NewInstance(argc, argv));
//     }
//   }
  
 


static Handle<Value> GetNymID(Local<String> property, const v8::AccessorInfo& info) {
  // Extract the C++ request object from the JavaScript wrapper.
  return v8::String::New(mainNymID.c_str());
}

static void SetNymID(Local<String> property, Local<Value> value, const AccessorInfo& info) {
  v8::String::Utf8Value nymIDUTF(value);
  mainNymID = std::string(*nymIDUTF);
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

Handle<Value> ResyncNym(const Arguments& args){
  HandleScope scope;
  int32_t requestNumber = 1;

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
    
  
  //Flush messages, then send a checkServerID message to get a request number
  OTAPI_Wrap::FlushMessageBuffer();
  int32_t checkServerIDRequestNumber = OTAPI_Wrap::checkServerID(mainServerID, nymID);
  std::cout << "########### checkServerIDRequestNumber : " << checkServerIDRequestNumber << std::endl;
    
  std::cout << "########### sleeping....";
  OTAPI_Wrap::Sleep(1000);
  std::cout << "done" << std::endl;
    
  std::cout << "########### getting the checkServerID reply....";
  std::string checkServerIDResponse = OTAPI_Wrap::PopMessageBuffer(int64_t(checkServerIDRequestNumber), mainServerID, nymID);
  std::cout << "success" << std::endl;
     
  std::cout << "########### getting the createNymRequest for this Nym....";  
  OT_ME madeEasy = OT_ME();
  std::string createNymRequest = madeEasy.register_nym(mainServerID, nymID);
    
  std::cout << "########### syncing the requestNumber with the Server....";  
  OTAPI_Wrap::FlushMessageBuffer();
  requestNumber = OTAPI_Wrap::getRequest(mainServerID, nymID);  
    
  std::cout << "########### resyncing the Nym with the Server...." << std::endl;  
  bool resyncSuccess = OTAPI_Wrap::ResyncNymWithServer(mainServerID, nymID, createNymRequest);  
  if(!resyncSuccess){
    std::cout << "########### RESYNCING FAILED...MAKING SURE EVERYTHING REQUIRED FOR SYNC MATCHES" << std::endl;  
    std::cout << "########### CHECKING NYMBOX FOR RECENT CHANGES..." << std::endl;     
    std::string strRecentHash = OTAPI_Wrap::GetNym_RecentHash(mainServerID, nymID);
    std::string strLocalHash = OTAPI_Wrap::GetNym_NymboxHash(mainServerID, nymID);
    if (strRecentHash != strLocalHash){ // the hashes do not match -- need to download           
      std::cout << "########### SERVER AND CLIENT DO NOT MATCH...RELOADING NYMBOX FROM SERVER..." << std::endl;    
      OTAPI_Wrap::FlushMessageBuffer();
      requestNumber = OTAPI_Wrap::getNymbox(mainServerID, nymID);
      OTAPI_Wrap::Sleep(1000);
      std::string strResponseMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(requestNumber), mainServerID, nymID);  
     
      std::string strServerHash = OTAPI_Wrap::Message_GetNymboxHash(strResponseMessage);
      strLocalHash = OTAPI_Wrap::GetNym_NymboxHash(mainServerID, nymID);
      if (strServerHash != strLocalHash){
	std::cout << "########### SERVER AND CLIENT STILL DO NOT MATCH...TRYING ONCE MORE" << std::endl;  
	// the getRequest worked, and the server hashes don't match, so let's try the call again...	
	OTAPI_Wrap::FlushMessageBuffer();
	requestNumber = OTAPI_Wrap::getNymbox(mainServerID, nymID);
	OTAPI_Wrap::Sleep(1000);
	strResponseMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(requestNumber), mainServerID, nymID);  	
      } 
    }         
            
    std::cout << "########### MAKING SURE ALL RECEIPTS ARE CURRENT..." << std::endl; 
    std::string ledger = OTAPI_Wrap::LoadNymboxNoVerify(mainServerID, nymID);
    OTAPI_Wrap::VerifySignature(nymID, ledger);
    int32_t nReceiptCount = OTAPI_Wrap::Ledger_GetCount(mainServerID, nymID, nymID, ledger);
    for (int32_t i_loop = 0; i_loop < nReceiptCount; ++i_loop){
      int64_t lTransactionNum = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(mainServerID, nymID, nymID, ledger, i_loop);
      if (lTransactionNum > 0){
	std::string strTransaction = OTAPI_Wrap::Ledger_GetTransactionByID(mainServerID, nymID, nymID, ledger, lTransactionNum);
	std::string strTransType = OTAPI_Wrap::Transaction_GetType(mainServerID, nymID, nymID, strTransaction);
	bool bIsReplyNotice = (strTransType == "replyNotice");
	int64_t boxReceiptRequestNumber = 0;
	if (bIsReplyNotice){
	  boxReceiptRequestNumber = OTAPI_Wrap::ReplyNotice_GetRequestNum(mainServerID, nymID, strTransaction);
	}
	
	bool bShouldDownload = (!bIsReplyNotice || (bIsReplyNotice && (0 < boxReceiptRequestNumber) && !OTAPI_Wrap::HaveAlreadySeenReply(mainServerID, nymID, boxReceiptRequestNumber)));
	if (bShouldDownload){
	  bool bHaveBoxReceipt = OTAPI_Wrap::DoesBoxReceiptExist(mainServerID, nymID, nymID, 0, lTransactionNum);
	  if (!bHaveBoxReceipt){
	    std::cout << "########### DOWNLOADING A RECEIPT FOR AN OUT-OF-DATE ACCOUNT..." << std::endl;  
		    
	    OTAPI_Wrap::FlushMessageBuffer();
	    int32_t nBoxReceiptRequestNum = OTAPI_Wrap::getBoxReceipt(mainServerID, nymID, nymID, 0, lTransactionNum);	
	    std::string strBoxReceiptMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(nBoxReceiptRequestNum), mainServerID, nymID); 
		    
	    std::cout << "########### BOX RECEIPT MESSAGE : " << strBoxReceiptMessage << std::endl;  
	  }
	}
      }
    }      
    std::cout << "########### BOX RECEIPT IS CURRENT" << std::endl;  
    
    std::cout << "########### TRYING AGAIN TO RESYNC THE NYM WITH THE SERVER...." << std::endl;  
    resyncSuccess = OTAPI_Wrap::ResyncNymWithServer(mainServerID, nymID, createNymRequest); 
  }
  
  
  
  //int32_t nGetNymbox = getNymbox(serverID, nymID, bForceDownload);
  std::cout << "########### checking nymbox for recent changes..." << std::endl;     
  std::string strRecentHash = OTAPI_Wrap::GetNym_RecentHash(mainServerID, nymID);
  std::string strLocalHash = OTAPI_Wrap::GetNym_NymboxHash(mainServerID, nymID);
  if (strRecentHash != strLocalHash){ // the hashes do not match -- need to download           
    std::cout << "########### SERVER AND CLIENT DO NOT MATCH...RELOADING NYMBOX FROM SERVER..." << std::endl;    
    OTAPI_Wrap::FlushMessageBuffer();
    requestNumber = OTAPI_Wrap::getNymbox(mainServerID, nymID);
    OTAPI_Wrap::Sleep(1000);
    std::string strResponseMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(requestNumber), mainServerID, nymID);  
    
    std::string strServerHash = OTAPI_Wrap::Message_GetNymboxHash(strResponseMessage);
    strLocalHash = OTAPI_Wrap::GetNym_NymboxHash(mainServerID, nymID);
    if (strServerHash != strLocalHash){
      std::cout << "########### SERVER AND CLIENT STILL DO NOT MATCH...TRYING ONCE MORE" << std::endl;  
      // the getRequest worked, and the server hashes don't match, so let's try the call again...	
      OTAPI_Wrap::FlushMessageBuffer();
      requestNumber = OTAPI_Wrap::getNymbox(mainServerID, nymID);
      OTAPI_Wrap::Sleep(1000);
      strResponseMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(requestNumber), mainServerID, nymID);  	
    } 
  }         
  
  //bool bInsured = insureHaveAllBoxReceipts(serverID, nymID, nymID, nBoxType, nRequestNumber, bFoundNymboxItem); 
  std::cout << "########### making sure all receipts are current..." << std::endl; 
  std::string ledger = OTAPI_Wrap::LoadNymboxNoVerify(mainServerID, nymID);
  OTAPI_Wrap::VerifySignature(nymID, ledger);
  int32_t nReceiptCount = OTAPI_Wrap::Ledger_GetCount(mainServerID, nymID, nymID, ledger);
  for (int32_t i_loop = 0; i_loop < nReceiptCount; ++i_loop){
    int64_t lTransactionNum = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(mainServerID, nymID, nymID, ledger, i_loop);
    if (lTransactionNum > 0){
      std::string strTransaction = OTAPI_Wrap::Ledger_GetTransactionByID(mainServerID, nymID, nymID, ledger, lTransactionNum);
      std::string strTransType = OTAPI_Wrap::Transaction_GetType(mainServerID, nymID, nymID, strTransaction);
      bool bIsReplyNotice = (strTransType == "replyNotice");
      int64_t boxReceiptRequestNumber = 0;
      if (bIsReplyNotice){
	boxReceiptRequestNumber = OTAPI_Wrap::ReplyNotice_GetRequestNum(mainServerID, nymID, strTransaction);
      }
      
      bool bShouldDownload = (!bIsReplyNotice || (bIsReplyNotice && (0 < boxReceiptRequestNumber) && !OTAPI_Wrap::HaveAlreadySeenReply(mainServerID, nymID, boxReceiptRequestNumber)));
      if (bShouldDownload){
	bool bHaveBoxReceipt = OTAPI_Wrap::DoesBoxReceiptExist(mainServerID, nymID, nymID, 0, lTransactionNum);
	if (!bHaveBoxReceipt){
	  std::cout << "########### DOWNLOADING A RECEIPT FOR AN OUT-OF-DATE ACCOUNT..." << std::endl;  
		  
	  OTAPI_Wrap::FlushMessageBuffer();
	  int32_t nBoxReceiptRequestNum = OTAPI_Wrap::getBoxReceipt(mainServerID, nymID, nymID, 0, lTransactionNum);	
	  std::string strBoxReceiptMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(nBoxReceiptRequestNum), mainServerID, nymID); 
		  
	  std::cout << "########### BOX RECEIPT MESSAGE : " << strBoxReceiptMessage << std::endl;  
	}
      }
    }
  }      
  std::cout << "########### box receipt is current" << std::endl;  
     
  
  std::cout << "########### cleanup transaction numbers and messages..." << std::endl; 
  if (requestNumber > 0){
    std::string strSentMsg = OTAPI_Wrap::GetSentMessage(int64_t(requestNumber), mainServerID, nymID);
    if(strSentMsg.length() > 0){
      OTAPI_Wrap::Msg_HarvestTransactionNumbers(strSentMsg, nymID, false, false, false, false, false);
      OTAPI_Wrap::RemoveSentMessage(int64_t(requestNumber), mainServerID, nymID);
    }
  }
  std::string strNymbox = OTAPI_Wrap::LoadNymboxNoVerify(mainServerID, nymID); 
  if (0 < strNymbox.length()){
    OTAPI_Wrap::FlushSentMessages(false, mainServerID, nymID, strNymbox);
  }
    
  std::cout << "########### processing the Nymbox..." << std::endl; 
  //int32_t nProcess = processNymbox(mainServerID, nymID, bWasMsgSent, -1, -1, -1, -1);
  OTAPI_Wrap::FlushMessageBuffer();
  int32_t processNymboxRequestNumber = OTAPI_Wrap::processNymbox(mainServerID, nymID);  
  if (processNymboxRequestNumber == 0){ // Nymbox was empty. Nothing to process.
    std::cout << "########### ALL SYNCING COMPLETE" << std::endl; 
    return scope.Close(True()); 
  } 
  if (processNymboxRequestNumber < 0){ // Error condition
    std::cout << "########### ERROR OCCURRED WHILE PROCESSING THE NYMBOX" << std::endl; 
    return scope.Close(False());    
  } 
  std::string processNymboxReplyMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(processNymboxRequestNumber), mainServerID, nymID); 
  int32_t nTransSuccess = -1;
  int32_t nBalanceSuccess = OTAPI_Wrap::Message_GetBalanceAgreementSuccess(mainServerID, nymID, nymID, processNymboxReplyMessage); 
  if (nBalanceSuccess > 0){
    std::cout << "########### successfully synced the balance for the Nymbox..." << std::endl; 
    nTransSuccess = OTAPI_Wrap::Message_GetTransactionSuccess(mainServerID, nymID, nymID, processNymboxReplyMessage); 
  }
  if (nTransSuccess > 0){ 
    std::cout << "########### successfully completed the transaction to sync the Nymbox..." << std::endl; 
  } 
    
  return scope.Close(True());
      
  
    
  
  
  
  
  /*  Utility::getNymbox - Condensed
  
  string strRecentHash = OTAPI_Wrap::GetNym_RecentHash(serverID, nymID);
  string strLocalHash = OTAPI_Wrap::GetNym_NymboxHash(serverID, nymID);
  if (strRecentHash == strLocalHash){ // the hashes match -- no need to download anything.
    OTAPI_Wrap::Output(1, "The hashes already match (skipping Nymbox download.)\n");
    return 1;
  }
  bool bWasMsgSent = false;
  int32_t nGetNymbox = getNymboxLowLevel(serverID, nymID, bWasMsgSent);
  string strServerHash = OTAPI_Wrap::Message_GetNymboxHash(strLastReplyReceived);
  strLocalHash = OTAPI_Wrap::GetNym_NymboxHash(serverID, nymID);
  if (bForceDownload || !bLocalHash || !bServerHash || (bServerHash && bLocalHash && !(strServerHash == strLocalHash))){
    // the getRequest worked, and the server hashes don't match,
    // so let's try the call again...
    nGetNymbox = getNymboxLowLevel(serverID, nymID, bWasMsgSent);
  }
    
    
  */
   
  
  
  /*  Utility::getNymboxLowLevel - Condensed
  
  OTAPI_Wrap::FlushMessageBuffer();
  int32_t nRequestNum = OTAPI_Wrap::getNymbox(serverID, nymID);
  OTAPI_Wrap::Sleep(50);
  string strResponseMessage = OTAPI_Wrap::PopMessageBuffer(int64_t(nRequestNum), serverID, nymID);  
  
  */
  
  
  
  /* Utility::insureHaveAllBoxReceipts - Condensed

  
  
  string ledger = OTAPI_Wrap::LoadNymboxNoVerify(serverID, nymID);
  OTAPI_Wrap::VerifySignature(nymID, ledger)
  int32_t nReceiptCount = OTAPI_Wrap::Ledger_GetCount(serverID, nymID, accountID, ledger);
  for (int32_t i_loop = 0; i_loop < nReceiptCount; ++i_loop){
    int64_t lTransactionNum = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(serverID, nymID, accountID, ledger, i_loop);
    if (lTransactionNum > 0){
      string strTransaction = OTAPI_Wrap::Ledger_GetTransactionByID(serverID, nymID, accountID, ledger, lTransactionNum);
      string strTransType = OTAPI_Wrap::Transaction_GetType(serverID, nymID, accountID, strTransaction);
      bool bIsReplyNotice = (VerifyStringVal(strTransType) && (strTransType == "replyNotice"));
      int64_t lRequestNum = 0;
      if (bIsReplyNotice){
	lRequestNum = OTAPI_Wrap::ReplyNotice_GetRequestNum(serverID, nymID, strTransaction);
      }
      
      bool bShouldDownload = (!bIsReplyNotice || (bIsReplyNotice && (0 < lRequestNum) && !OTAPI_Wrap::HaveAlreadySeenReply(serverID, nymID, lRequestNum)));
      if (bShouldDownload){ // This block executes if we should download it (assuming we haven't already, which it also checks for.)
	bool bHaveBoxReceipt = OTAPI_Wrap::DoesBoxReceiptExist(serverID, nymID, accountID, nBoxType, lTransactionNum);
	if (!bHaveBoxReceipt){
	  OTAPI_Wrap::Output(1, strLocation + ": Downloading box receipt to add to my collection...\n");
	  bool bDownloaded = getBoxReceiptWithErrorCorrection(serverID, nymID, accountID, nBoxType, lTransactionNum);
	  if (!bDownloaded){
	    OTAPI_Wrap::Output(0, strLocation + ": Failed downloading box receipt. (Skipping any others.) Transaction number: " + to_string(lTransactionNum) + "\n");
	    bReturnValue = false;
	    break;
	  }
	}
      }
    }
  }
  
  
      
   */
  
  
  
  /*  Utility::getAndProcessNymbox_8 - Condensed
  
    //getAndProcessNymbox_8(serverID, nymID, OTBool(false), true, 0, false, false, F/F/F/F);
  
    int32_t nGetNymbox = getNymbox(serverID, nymID, bForceDownload);
    bool bInsured = insureHaveAllBoxReceipts(serverID, nymID, nymID, nBoxType, nRequestNumber, bFoundNymboxItem); 
    if (nRequestNumber > 0){
      string strSentMsg = OTAPI_Wrap::GetSentMessage(int64_t(nRequestNumber), serverID, nymID);
      bool nHarvested = OTAPI_Wrap::Msg_HarvestTransactionNumbers(strSentMsg, nymID,
	  bHarvestingForRetry, // bHarvestingForRetry.
	  bMsgReplySuccess, // bReplyWasSuccess, // RECEIVED server reply: explicit success.
	  bMsgReplyFailure, // bReplyWasFailure, // RECEIVED server reply: explicit failure.
	  bMsgTransSuccess, // bTransactionWasSuccess, // MESSAGE success, Transaction success. (Explicit.)
	  bMsgTransFailure); // bTransactionWasFailure // MESSAGE success, Transaction failure. (Explicit.)
      bool nRemovedMsg = OTAPI_Wrap::RemoveSentMessage(int64_t(nRequestNumber), serverID, nymID);
    }
    string strNymbox = OTAPI_Wrap::LoadNymboxNoVerify(serverID, nymID); // FLUSH SENT MESSAGES!!!! (AND HARVEST.);
    if (0 < strNymbox.length())){
      OTAPI_Wrap::FlushSentMessages(false, //harvesting for retry = = OT_FALSE. None of the things are being re-tried by the time they are being flushed. They were already old news.;
	serverID,
	nymID,
	strNymbox);
    }
    int32_t nProcess = processNymbox(serverID, nymID, bWasMsgSent, nMsgSentRequestNumOut, nReplySuccessOut, nBalanceSuccessOut, nTransSuccessOut);
    
    string strReplyProcess = getLastReplyReceived();
    
  
  */
  
  
  
  /*  Utility::processNymbox - Condensed
  
  OTAPI_Wrap::FlushMessageBuffer();
  int32_t nRequestNum = OTAPI_Wrap::processNymbox(serverID, nymID);  
  if (nRequestNum == 0){ return 0; } // Nymbox was empty. Nothing to process.
  if (nRequestNum < 0){ return -1; } // Error condition
  string strReplyProcess = OTAPI_Wrap::PopMessageBuffer(int64_t(nRequestNum), serverID, nymID); 
  int32_t nTransSuccess = -1;
  int32_t nBalanceSuccess = OTAPI_Wrap::Message_GetBalanceAgreementSuccess(serverID, nymID, nymID, strReplyProcess); 
  if (nBalanceSuccess > 0){
    nTransSuccess = OTAPI_Wrap::Message_GetTransactionSuccess(serverID, nymID, nymID, strReplyProcess); 
  }
  if (nTransSuccess > 0){ return nRequestNum; }
  
  */
   
      
  /* resync.ot - Condensed
  OT_API_FlushMessageBuffer();
  var nCheckServerID = OT_API_checkServerID(temp_Server, temp_MyNym);
  OT_API_Sleep(int64_t(100));
  var strServerReply = OT_API_PopMessageBuffer(int64_t(nCheckServerID), temp_Server, temp_MyNym);
  var strCreateUserReply = madeEasy.register_nym(temp_Server, temp_MyNym)
  MsgUtil.getRequestNumber(temp_Server, temp_MyNym)
  var bResyncNymWithServer = OT_API_ResyncNymWithServer(temp_Server, temp_MyNym, strCreateUserReply)
  if (!bResyncNymWithServer){
    var nGetNymbox = MsgUtil.getNymbox(temp_Server, temp_MyNym, true) //bForceDownload=true
    MsgUtil.insureHaveAllBoxReceipts(temp_Server, temp_MyNym, temp_MyNym, 0)
    bResyncNymWithServer = OT_API_ResyncNymWithServer(temp_Server, temp_MyNym, strCreateUserReply)
  }
  var bWasMsgSent = OTBool(false)
  var nGetAndProcessNymbox = MsgUtil.getAndProcessNymbox_4(temp_Server, temp_MyNym, bWasMsgSent, true)
  */
  
   
  
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




Handle<Value> SignNewAssetContract(const Arguments& args){
  HandleScope scope;

  //Make sure all the arguments have been supplied
  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected: Nym ID, Unsigned Asset Contract XML")));
    return scope.Close(False());
  }

  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (Nym ID) is not in the correct format.")));
    return scope.Close(False());
  }

  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Second parameter (Unsigned Asset Contract XML) is not in the correct format.")));
    return scope.Close(False());
  }
  
  //Extract the values supplied as arguments
  v8::String::Utf8Value nymIDUTF(args[0]->ToString());
  std::string nymID = std::string(*nymIDUTF);
  const OTIdentifier signingID(nymID);
  OTPseudonym* signingNym = OTAPI_Wrap::OTAPI()->GetOrLoadPublicNym(signingID, __FUNCTION__); 
  OTPasswordData thePWData("test");
  signingNym->LoadCredentials(true, &thePWData);
  signingNym->VerifyPseudonym();
  
  v8::String::Utf8Value unsignedAssetContractXMLUTF(args[1]->ToString());
  std::string unsignedAssetContractXML = std::string(*unsignedAssetContractXMLUTF);
  OTString unsignedXML(unsignedAssetContractXML);
  
  //Get the create the asset contract from XML, add the nym and public key, and sign with the public key
  OTString strPubkey;
  bool pubKeyLoaded = signingNym->GetPublicSignKey().GetPublicKey(strPubkey);
  OTAssetContract* newAssetContract = new OTAssetContract(unsignedXML);  
    
  const bool bHasCredentials = (signingNym->GetMasterCredentialCount() > 0);
  std::cout << "Has Credentials = " << bHasCredentials << std::endl;
  if (!bHasCredentials){
      if (pubKeyLoaded && strPubkey.Exists()) {  //bEscaped=true by default.	  
	  newAssetContract->InsertNym("contract", strPubkey);
      }
  } else {// signingNym has Credentials, so we'll add him to the contract.
      OTString     strCredList, strSignerNymID;
      mapOfStrings mapCredFiles;
      signingNym->GetIdentifier(strSignerNymID);
      signingNym->GetPublicCredentials(strCredList, &mapCredFiles);
      
      OTPseudonym * pNym = new OTPseudonym;
      OTCleanup<OTPseudonym> theNymAngel(pNym); // pNym will be automatically cleaned up.
      
      pNym->SetIdentifier(strSignerNymID);
      pNym->SetNymIDSource(signingNym->GetNymIDSource());
      pNym->SetAltLocation(signingNym->GetAltLocation());
      
      if (false == pNym->LoadFromString(strCredList, &mapCredFiles)) {
	  //OTLog::vError("%s: Failure loading nym %s from credential string.\n", __FUNCTION__, strSignerNymID.Get());
      } else if (false == pNym->VerifyPseudonym()) {
	  // Now that the Nym has been loaded up from the two strings,
	  // including the list of credential IDs, and the map containing the
	  // credentials themselves, let's try to Verify the pseudonym. If we
	  // verify, then we're safe to add the Nym to the contract.
	  //
	  //OTLog::vError("%s: Loaded nym %s from credentials, but then it failed verifying.\n", __FUNCTION__, strSignerNymID.Get());
      } else { 
	  // Okay, we loaded the Nym up from the credentials, AND
	  // verified the Nym (including the credentials.)
	  // So let's add it to the contract...
	  theNymAngel.SetCleanupTargetPointer(NULL); // so pNym won't be cleaned up.
	  if (pubKeyLoaded && strPubkey.Exists()) {  //bEscaped=true by default.	  
	      newAssetContract->InsertNym("signer", strPubkey);  // Add pNym to the contract's internal list of nyms.
	  }  
      }
  }
    
  //Generate the contract contents, save it out, and sign it
  OTString assembledContract, signedContract, contractType("CONTRACT");
  newAssetContract->CreateContents();
  newAssetContract->SignContract(*signingNym);
  //newAssetContract->RewriteContract(signedContract);
  newAssetContract->SaveContents(assembledContract);  
  newAssetContract->SignFlatText(assembledContract, contractType, *signingNym, signedContract);
  
  //Sign the contract
  //newAssetContract->SignFlatText(signedContract, 
  
  std::string assetID = signedContract.Get();
  
  return scope.Close(v8::String::New(assetID.c_str()));
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





Handle<Value> CreateSmartContract(const Arguments& args){
  HandleScope scope;
  
  OTString serverIDOT(mainServerID);
  OTIdentifier serverID(mainServerID);
  
  //Make sure all the arguments have been supplied
  if (args.Length() < 5) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments. Expected: Signing Nym ID, Receiver Nym ID, Asset Type ID, Amount, and Original Deposit Account ID")));
    return scope.Close(False());
  }
  
  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First parameter (Signing Nym ID) is not in the correct format.")));
    return scope.Close(False());
  }
  
  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Second parameter (Receiver Nym ID) is not in the correct format.")));
    return scope.Close(False());
  }
  
  if (!args[2]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Third parameter (Asset Type ID) is not in the correct format.")));
    return scope.Close(False());
  }
  
  if (!args[3]->IsNumber()) {
    ThrowException(Exception::TypeError(String::New("Fourth parameter (Amount) is not in the correct format.")));
    return scope.Close(False());
  }
  
  if (!args[4]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Fifth parameter (Original Deposit Account ID) is not in the correct format.")));
    return scope.Close(False());
  }
  
  
  
  
  
  
  
  
  
  //Extract the values supplied as arguments
  v8::String::Utf8Value signingNymIDUTF(args[0]->ToString());
  std::string signingNymID = std::string(*signingNymIDUTF);
  //const OTString signingNymIDOT(signingNymID.c_str());
  //OTPseudonym signingPseudonym(signingNymIDOT);
  
  v8::String::Utf8Value receivingNymIDUTF(args[1]->ToString());
  std::string receivingNymID = std::string(*receivingNymIDUTF);
  
  v8::String::Utf8Value assetTypeIDUTF(args[2]->ToString());
  std::string assetTypeID = std::string(*assetTypeIDUTF);
  const OTString assetTypeIDOT(assetTypeID.c_str());
  
  int32_t initialBalanceAmount = args[3]->IntegerValue();
  
  v8::String::Utf8Value originalDepositAccountIDUTF(args[4]->ToString());
  std::string originalDepositAccountID = std::string(*originalDepositAccountIDUTF);
    
  const OTIdentifier signingID(signingNymID);
  OTPseudonym* privateSigningNym = OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(signingID, false, __FUNCTION__); 
  OTPseudonym* privateReceiverNym = OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(receivingNymID, false, __FUNCTION__); 
  //OTPseudonym* publicSigningNym = OTAPI_Wrap::OTAPI()->GetOrLoadPublicNym(signingID, __FUNCTION__);
    
  OTString nymName = privateSigningNym->GetNymName();
  std::cout << "Signing NYM = " << nymName.Get() << std::endl;
      
  int32_t issuedTransactionNumberCount = privateSigningNym->GetTransactionNumCount(serverID);
  std::cout << "Transaction Numbers: " << issuedTransactionNumberCount << std::endl;
  
  
  
  /*	CREATE A SMART CONTRACT USING OTAPI_WRAP
  const std::string signingNym = std::string(*signingNymIDUTF);
  //const time64_t validFrom = 0;
  //const time64_t validTo = 0;
  
  //std::string newContract = OTAPI_Wrap::Create_SmartContract(signingNym, validFrom, validTo);
  //std::cout << "New Contract = " << newContract << std::endl;
  
  //newContract = OTAPI_Wrap::SmartContract_AddParty(newContract, signingNym, "Issuer_Party", "Issuer_Agent");
  //std::cout << "Contract with Issuer = " << newContract << std::endl;
  
  //newContract = OTAPI_Wrap::SmartContract_AddAccount(newContract, signingNym, "Issuer_Party", "Original_Deposit_Account", assetTypeID);
  //std::cout << "Contract with Issuer Account = " << newContract << std::endl;
  
  newContract = OTAPI_Wrap::SmartContract_AddParty(newContract, signingNym, "Receiver_Party", "Receiver_Agent");
  //std::cout << "Contract with Receiver = " << newContract << std::endl;  
  
  newContract = OTAPI_Wrap::SmartContract_AddAccount(newContract, signingNym, "Receiver_Party", "Receiver_Account", assetTypeID);
  //std::cout << "Contract with Receiver Account = " << newContract << std::endl;
      
  newContract = OTAPI_Wrap::SmartContract_AddBylaw(newContract, signingNym, "Main_Bylaw");
  //std::cout << "Contract with Bylaw = " << newContract << std::endl;
  
  newContract = OTAPI_Wrap::SmartContract_AddClause(newContract, signingNym, "Main_Bylaw", "Main_Clause", "var testVariable = true");
  //std::cout << "Contract with Clause = " << newContract << std::endl;
  
  newContract = OTAPI_Wrap::SmartContract_AddHook(newContract, signingNym, "Main_Bylaw", "cron_activate", "Main_Clause");
  std::cout << "Contract before Confirm = " << newContract << std::endl;
  
  
  
  //Confirm everything  
  newContract = OTAPI_Wrap::SmartContract_ConfirmParty(newContract, "Issuer_Party", signingNymID);
  std::cout << "Contract with Confirmed Issuer = " << newContract << std::endl;
  
  newContract = OTAPI_Wrap::SmartContract_ConfirmParty(newContract, "Receiver_Party", receivingNymID);
  std::cout << "Contract with Confirmed Receiver = " << newContract << std::endl;
  
  newContract = OTAPI_Wrap::SmartContract_ConfirmAccount(newContract, signingNym, "Issuer_Party", "Original_Deposit_Account", "Issuer_Agent", originalDepositAccountID);
  std::cout << "Contract with Confirmed Issuer Account = " << newContract << std::endl;
  
  */
  
  
  
  
  //	CREATE A SMART CONTRACT USING OTSmartContract
  OTString newSmartContractText;
  
  OTSmartContract* newSmartContract = new OTSmartContract(serverID);
  newSmartContract->SetDateRange(0, 0);
  
  //Add issuer party and agent
  OTParty* issuerParty = new OTParty("Issuer_Party", true, nullptr, "Issuer_Agent", true);
  newSmartContract->AddParty(*issuerParty);
  
  //Add receiver party and agent
  OTParty* receiverParty = new OTParty("Receiver_Party", true, nullptr, "Receiver_Agent", true);
  newSmartContract->AddParty(*receiverParty);
  
  //Add the Main Bylaw 
  OTBylaw* mainBylaw = new OTBylaw("Main_Bylaw", "chai");
  
  //Add Activation Clause and Hook
  std::string activationClauseCode = ""
  "print(\"SMART CONTRACT: OnActivate(), (from cron_activate hook)\"); "
  "var initialStashWorked = false;"
  "try{"
  //Check the Initial_Deposit_Account for available funds" 
  "  var currentInitialBalance = Initial_Balance;" 
  "  var availableDepositBalance = get_acct_balance(\"Initial_Deposit_Account\").to_int();" 
  "  print(\"SMART CONTRACT: Initial_Balance for this contract = ${currentInitialBalance}\");" 
  "  print(\"SMART CONTRACT: Initial_Deposit_Account balance = ${availableDepositBalance}\");" 
  ""
  "  var fundsAreAvailable = (availableDepositBalance >= currentInitialBalance);" 
  "  print(\"SMART CONTRACT: Funds Are Available = ${fundsAreAvailable}\");" 
  "  if(!fundsAreAvailable){" 
  "    print(\"SMART CONTRACT: The Initial_Deposit_Account does not contain funds to cover the initial balance transfer.\");" 
  "    deactivate_contract();" 
  "    return;" 
  "  }" 
  //Stash the funds from the Initial_Deposit_Account into the Receiver_Stash_Account" 
  "  initialStashWorked = stash_funds(\"Initial_Deposit_Account\", \"Receiver_Stash_Account\", Initial_Balance.to_string());"
  "  availableDepositBalance = get_acct_balance(\"Initial_Deposit_Account\").to_int();" 
  "  print(\"SMART CONTRACT: Initial_Deposit_Account balance = ${availableDepositBalance}\");" 
  "}catch(e){" 
  "  if(initialStashWorked){ "
  "    unstash_funds(\"Initial_Deposit_Account\", \"Receiver_Stash_Account\", Initial_Balance.to_string());"
  "    availableDepositBalance = get_acct_balance(\"Initial_Deposit_Account\").to_int();" 
  "    print(\"SMART CONTRACT: Initial_Deposit_Account balance = ${availableDepositBalance}\");" 
  "  }"
  "  deactivate_contract();" 
  "  print(\"SMART CONTRACT: Error during Activation Clause\");" 
  "  return;" 
  "}" 
  ""
  "set_seconds_until_timer(\"60\");";
  
  mainBylaw->AddClause("Activation_Clause", activationClauseCode.c_str());
  mainBylaw->AddHook("cron_activate", "Activation_Clause");
  
  //Add Process Clause and Hook    
  std::string processClauseCode = ""
  "print(\"SMART CONTRACT: OnProcess\");"
  "var currentInitialBalance = Initial_Balance;"  
  "try { "
  "  var currentReceiverStashBalance = get_acct_balance(\"Initial_Deposit_Account\");"  
  "  eval(\"function decay(){print(\" + currentReceiverStashBalance + \");}decay();\");" 
  "} catch(e) {"
  "  print(\"SMART CONTRACT: Error during Update_Balance_Clause\");"
  "  print(\"SMART CONTRACT: \" + e.what());"
  
  "}"
  
  "    unstash_funds(\"Initial_Deposit_Account\", \"Receiver_Stash_Account\", Initial_Balance.to_string());"
  "    var depositAccountBalance = get_acct_balance(\"Initial_Deposit_Account\").to_int();" 
  "    print(\"SMART CONTRACT: Deposit Account balance = ${depositAccountBalance}\");" 
  "deactivate_contract();"
  "";
  
  mainBylaw->AddClause("Update_Balance_Clause", processClauseCode.c_str());
  mainBylaw->AddHook("cron_process", "Update_Balance_Clause");
  
  //Add Callbacks and their Clauses
  //mainBylaw->AddClause("Cancel_Contract_Check", "var testVariable = true;");
  //mainBylaw->AddCallback("", "Cancel_Contract_Check");
  
  //mainBylaw->AddClause("Change_Receiver", "var testVariable = true;");
  //mainBylaw->AddCallback("", "Change_Receiver");
  
  //Add all Variables
  mainBylaw->AddVariable("Initial_Balance", initialBalanceAmount);
  mainBylaw->AddVariable("Decay_Function", "");    
  mainBylaw->AddVariable("Pending_Transfer", false);
  mainBylaw->AddVariable("Transfer_Amount", 0);
  mainBylaw->AddVariable("New_Receiver", "");
  
  newSmartContract->AddBylaw(*mainBylaw);
  
  
  
  
  
  
  //Add the Issuer Deposit Account
  const OTString issuerAgentName("Issuer_Agent"), issuerInitialDepositAcctName("Initial_Deposit_Account"), issuerAccountID(originalDepositAccountID.c_str());
  issuerParty->AddAccount(issuerAgentName, issuerInitialDepositAcctName, issuerAccountID, assetTypeIDOT, 0);
    
  //Confirm the Issuer Party, Receiver Party, and Issuer Deposit Account    
  OTParty* confirmedIssuerParty = new OTParty(issuerParty->GetPartyName(), *privateSigningNym, issuerParty->GetAuthorizingAgentName());
  issuerParty->CopyAcctsToConfirmingParty(*confirmedIssuerParty); 
  
  OTParty* confirmedReceiverParty = new OTParty(receiverParty->GetPartyName(), *privateReceiverNym, receiverParty->GetAuthorizingAgentName());
  receiverParty->CopyAcctsToConfirmingParty(*confirmedReceiverParty); 
    
  newSmartContract->ConfirmParty(*confirmedIssuerParty);
  newSmartContract->ConfirmParty(*confirmedReceiverParty);
    
  //newSmartContract->SignContract(*privateSigningNym); 
  newSmartContract->SaveContract();   
  newSmartContract->SaveContractRaw(newSmartContractText);
  
  
  
  
  //Activate the Smart Contract on the server
  int returnValue = OTAPI_Wrap::OTAPI()->activateSmartContract(serverID, signingID, newSmartContractText);
  
  
  
  
  
  //Logging for Transaction Numbers
  //int32_t remainingTransactionNumberCount = 0;
  //int64_t nextAvailableTransactionNumber = 0;
  //if(issuedTransactionNumberCount > 0){
  // int64_t currentIssuedTransactionNumber = 0;
  // for(int32_t i = 0; i < issuedTransactionNumberCount; i++){
  //   currentIssuedTransactionNumber = privateSigningNym->GetTransactionNum(serverID, i);
     //privateSigningNym->ClawbackTransactionNumber(serverID, currentIssuedTransactionNumber, true, nullptr);
     //privateSigningNym->HarvestIssuedNumbers(serverID, *privateSigningNym, *privateSigningNym, true);
     //privateSigningNym->HarvestTransactionNumbers(serverID, *privateSigningNym, *privateSigningNym, true);
     
     //privateSigningNym->GetNextTransactionNum(*privateSigningNym, serverIDOT, nextAvailableTransactionNumber, false);
  //   remainingTransactionNumberCount = privateSigningNym->GetTransactionNumCount(serverID);
     
  //   std::cout << "Transaction " << i << ": " << currentIssuedTransactionNumber << " : Next Trans #: " << nextAvailableTransactionNumber << " : Trans # Count: " << remainingTransactionNumberCount << std::endl;
  // }    
  //} 
  
  //int64_t nextAvailableTransactionNumber = 0;
  //privateSigningNym->GetNextTransactionNum(*privateSigningNym, serverIDOT, nextAvailableTransactionNumber, true);
  //privateSigningNym->AddTransactionNum(*privateSigningNym, serverIDOT, nextAvailableTransactionNumber, false);
  //std::cout << "Next Transaction Number: " << nextAvailableTransactionNumber << std::endl;
  
  //int32_t firstAvailableTransactionNumber = privateSigningNym->GetTransactionNum(serverID, issuedTransactionNumberCount-1);
  //issuerParty->SetOpeningTransNo(firstAvailableTransactionNumber); 
  //issuerParty->ReserveTransNumsForConfirm(serverIDOT);  
  
  
  
  
  
  
  //Attempt at Loading SmartContracts from XML 
  //if (!args[0]->IsString()) {
  //  ThrowException(Exception::TypeError(String::New("First parameter (Smart Contract XML) is not in the correct format.")));
  //  return scope.Close(False());
  //}
  //v8::String::Utf8Value smartContractXMLUTF(args[0]->ToString());
  //std::string smartContractXML = std::string(*smartContractXMLUTF);
  //OTString smartContractXMLOT(smartContractXML.c_str());
  //OTSmartContract smartContract = OTSmartContract();
  //bool isContractLoaded = smartContract.CreateContract(smartContractXMLOT, *publicSigningNym);  
  //std::cout << "Loaded Contract? = " << isContractLoaded << std::endl;
  
  
  
  
 //CODE from OTContract.cpp -> LoadContractXML 
 /*   
  
  if (!m_xmlUnsigned.Exists()){
    return false;
  }
  m_xmlUnsigned.reset();
  IrrXMLReader* xml = irr::io::createIrrXMLReader(m_xmlUnsigned);
  OT_ASSERT_MSG(NULL != xml, "Memory allocation issue with xml reader in OTContract::LoadContractXML()\n");
  OTCleanup<IrrXMLReader> xmlAngel(*xml);
  
  // parse the file until end reached
  while(xml->read()){
    OTString strNodeType;
    switch(xml->getNodeType()){
      case EXN_NONE: goto switch_log;
      case EXN_COMMENT: goto switch_log;
      case EXN_ELEMENT_END: goto switch_log;
      case EXN_CDATA: goto switch_log;
      case EXN_TEXT: goto switch_log;
      switch_log:
	break;
      case EXN_ELEMENT:	{
	retProcess = ProcessXMLNode(xml);
	// an error was returned. file format or whatever.
	if ((-1) == retProcess){
	  OTLog::Error("OTContract::LoadContractXML: (Cancelling this contract load; an error occurred.)\n");
	  return false;
	} else if (0 == retProcess){ // No error, but also the node wasn't found...
	  OTLog::vError("UNKNOWN element type in OTContract::LoadContractXML: %s, value: %s\n",
	    xml->getNodeName(), xml->getNodeData());
	}
      }
      break;
    default:
      continue;
    }
  }
  
  return true;
  
  */
  //END CODE from OTContract.cpp -> LoadContractXML



  
  return scope.Close(v8::String::New(newSmartContractText.Get()));
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
  exports->Set(String::NewSymbol("resyncNym"),
      FunctionTemplate::New(ResyncNym)->GetFunction());   
  exports->Set(String::NewSymbol("createAccount"),
      FunctionTemplate::New(CreateAccount)->GetFunction()); 
  exports->Set(String::NewSymbol("deleteAccount"),
      FunctionTemplate::New(DeleteAccount)->GetFunction());   
  exports->Set(String::NewSymbol("getAccountBalance"),
      FunctionTemplate::New(GetAccountBalance)->GetFunction());
  
  
  
  
  
  exports->Set(String::NewSymbol("transferAssets"),
      FunctionTemplate::New(TransferAssets)->GetFunction());
  
  
  exports->Set(String::NewSymbol("signNewAssetContract"),
      FunctionTemplate::New(SignNewAssetContract)->GetFunction());
  exports->Set(String::NewSymbol("createNewAsset"),
      FunctionTemplate::New(CreateNewAsset)->GetFunction());
  exports->Set(String::NewSymbol("issueAsset"),
      FunctionTemplate::New(IssueAsset)->GetFunction());
  exports->Set(String::NewSymbol("getSignedAssetContract"),
      FunctionTemplate::New(GetSignedAssetContract)->GetFunction());
  
  
  
  exports->Set(String::NewSymbol("createSmartContract"),
      FunctionTemplate::New(CreateSmartContract)->GetFunction());
  
  
  
  exports->SetAccessor(String::New("mainNymID"), GetNymID, SetNymID);
  exports->SetAccessor(String::New("mainServerID"), GetServerID, SetServerID);

}

NODE_MODULE(otapi, init)
