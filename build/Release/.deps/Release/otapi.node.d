cmd_Release/otapi.node := ln -f "Release/obj.target/otapi.node" "Release/otapi.node" 2>/dev/null || (rm -rf "Release/otapi.node" && cp -af "Release/obj.target/otapi.node" "Release/otapi.node")
