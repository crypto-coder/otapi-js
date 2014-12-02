var otapi = null;
 
// Load the precompiled binary for windows.
if(process.platform == "win32" && process.arch == "x64") {
  //otapi = require('./bin/winx64/otapi');
} else if(process.platform == "win32" && process.arch == "ia32") {
  //otapi = require('./bin/winx86/otapi');
} else {
  // Load the new built binary for other platforms.
  otapi = require('./build/Release/otapi');
}
 
module.exports = otapi;
