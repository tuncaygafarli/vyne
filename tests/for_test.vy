z = through a::1..30 -> filter {
    a % 2 == 0;
};

log(z);