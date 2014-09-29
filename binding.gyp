{
  "targets": [
    {
      "target_name": "node_otapi",
      "include_dirs" : [ "/home/ozymandias/dev/Open-Transactions/include/otapi", "/home/ozymandias/dev/Open-Transactions/include/otlib", "/home/ozymandias/dev/Open-Transactions/include", "/home/ozymandias/dev/node-otapi", "/usr/local/include/opentxs" ],
      "sources": [ "node_otapi.cc", "passwordCallback.cpp" ],
      "link_settings": { "libraries": [ "/usr/local/lib/libotapi.so", "/usr/local/lib/libot.so" ] },
      "cflags": [ "-std=c++11", "-fexceptions" ],
      "cflags_cc": [ "-fexceptions" ]
    }
  ]
}
