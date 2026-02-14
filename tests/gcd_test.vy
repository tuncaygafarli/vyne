module vcore;

# Greatest Common Divisor (GCD) Test

while true {
    input_1 = number(vcore.input("Enter number 1 : "));
    input_2 = number(vcore.input("Enter number 2 : "));

    a = input_1;
    b = input_2;
    
    while b > 0 || b < 0 {
        temp = b;
        b = a % b;
        a = temp; # This is the Euclidean algorithm we all learn in 6th grade LMAO
    }
    
    log(a);

    if input_1 == "exit" || input_2 == "exit" { break; }
}
