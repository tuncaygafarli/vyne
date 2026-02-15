module vcore;

# Quadratic Formula Test
# I have no idea how to find the square root of a number in vyne
# so I will implement the Newton-Raphson method to find the square root of the discriminant

while true {

    a_input = vcore.input("Enter a (or exit): ");
    if a_input == "exit" { break; }

    b_input = vcore.input("Enter b : ");
    c_input = vcore.input("Enter c : ");

    if b_input == "exit" || c_input == "exit" { break; }

    a = number(a_input);
    b = number(b_input);
    c = number(c_input);

    discriminant = b*b - 4*a*c;

    if discriminant < 0 {

        out("No real roots");

    } else {

        x = discriminant;
        i = 0;

        while i < 10 {

            x = (x + discriminant / x) / 2;

            i = i + 1;
        }

        sqrt_discriminant = x;

        root1 = (-b + sqrt_discriminant) / (2*a);
        root2 = (-b - sqrt_discriminant) / (2*a);

        out(root1);
        out(root2);
    }

}
