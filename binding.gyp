{
  "targets": [
    {
      "target_name": "otapi",
      "include_dirs" : [ ".", "/usr/local/include/opentxs" ],
      "sources": [ "otapi-js.cc", "passwordCallback.cpp" ],
      "link_settings": { "libraries": [ "/usr/local/lib/libotapi.so", "/usr/local/lib/libot.so" ] },
      "cflags": [ "-std=c++11", "-fexceptions" ],
      "cflags_cc": [ "-fexceptions" ]
    }
  ]
}
