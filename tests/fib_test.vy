n :: Number = 40;
a = 0;
b = 1;
i = 0;

log("Calculating Fibonacci(40)...");

while (i < n) {
    temp = a;
    a = b;
    b = temp + b;
    i++;
}

log("Result:");
log(a);