{
  "targets": [
    {
      "target_name": "node_otapi",
      "include_dirs" : [ "/home/ubuntu/dev/Open-Transactions/include/otapi", "/home/ubuntu/dev/Open-Transactions/include/otlib", "/home/ubuntu/dev/Open-Transactions/include", "/home/ubuntu/dev/nodejs/node_otapi" ],
      "sources": [ "node_otapi.cc", "passwordCallback.cpp" ],
      "link_settings": { "libraries": [ "/usr/local/lib/libotapi.so", "/usr/local/lib/libot.so" ] },
      "cflags": [ "-std=c++11", "-fexceptions" ],
      "cflags_cc": [ "-fexceptions" ]
    }
  ]
}
