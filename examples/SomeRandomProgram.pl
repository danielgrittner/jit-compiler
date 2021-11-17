PARAM x, y, z;
VAR a, b, c;
CONST A = 10, B = 15;

BEGIN
    a := x * y + B;
    b := z / y - A;
    c := (a + b) / 2;
    RETURN -c
END.
