#ifndef OTAPIJS_H
#define OTAPIJS_H

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>

class otapi_js : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
   
   
 private:
  explicit otapi_js();
  ~otapi_js();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  
  
  int32_t getNymbox(const std::string & nymID, const std::string & accountID);
  void ensureReceiptsAreCurrent(const std::string & nymID, const std::string & accountID);
  
  
  
  std::string mainNymID;
  std::string mainServerID;
};

#endif
