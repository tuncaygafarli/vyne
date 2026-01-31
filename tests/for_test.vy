data = [];
data.place_all(10, 5000); 

transformed = through it::1..data.size() -> collect {
    (it * 2 // 3) % 10 + 5;
};

evens = through transformed -> filter {
    it % 2 == 0;
};

total = 0;
through evens -> loop {
    total = total + it;
};

log(total);