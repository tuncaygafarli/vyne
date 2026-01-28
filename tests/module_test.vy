module vcore;

os = vcore.platform();
version = vcore.version;

stringTest = vcore.string(2);
log("Hello from " + os + " " + version + " " + stringTest);