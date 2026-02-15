bigArray = [];
i = 0;
while (i < 1000) {
    bigArray.push(i);
    i = i + 1;
}

out("Array built. Starting the stress test...");

start = 0;
while (start < 5000) {
    temp = bigArray; 
    
    start = start + 1;
}

out("Stress test finished!");