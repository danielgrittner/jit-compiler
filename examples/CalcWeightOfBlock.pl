PARAM width, height, depth;
VAR volume;
CONST density = 2400;

BEGIN
    volume := width * height * depth;
    RETURN density * volume
END.