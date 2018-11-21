

function CalcRGBPosition(rgb) {
    // rgbの値を3つの領域に分けるためのパーティションパラメータ
    const partitionMin = 85;
    const partitionMax = partitionMin * 2;
    let result;
    if (rgb < partitionMin) {
        result = 0;
    }
    else if (rgb < partitionMax) {
        result = 1;
    }
    else {
        result = 2;
    }
    return result;
}

function get_color_sensor(next) {
    // Red, Green, Blue
    var code = $('#colorPicker').val();
    const red = parseInt(code.substring(1, 3), 16);
    const green = parseInt(code.substring(3, 5), 16);
    const blue = parseInt(code.substring(5, 7), 16);

    $('#box').css('background-color', 'rgb(' + red + ', ' + green + ', ' + blue + ')');
    const redPosition = CalcRGBPosition(red);
    const greenPosition = CalcRGBPosition(green);
    const bluePosition = CalcRGBPosition(blue);
    let color;
    // Blue(0, 0, 255), LightBlue(0, 255, 255), Pink(255, 0, 255), Red(255, 0, 0), LightGreen(0, 255, 0), Green(0, 128, 0), Yellow(255, 255, 0), Black(0, 0, 0), 
    // White(255, 255, 255)
    next(redPosition, greenPosition, bluePosition);
}

function outputColor(redPosition, greenPosition, bluePosition) {

    console.log(redPosition * 100 + greenPosition * 10 + bluePosition);
    switch (redPosition * 100 + greenPosition * 10 + bluePosition) {
        case 000:
            color = 'Black';
            break;
        case 100:
        case 200:
            color = 'Red';
            break;
        case 10:
        case 11:
        case 20:
        case 21:
        case 120:
            color = 'Green';
            break;
        case 001:
        case 002:
            color = 'Blue';
            break;

        case 22:
        case 12:
            color = 'LightBlue';
            break;

        case 201:
        case 202:
        case 212:
        case 101:
        case 102:
            color = 'Pink';
            break;

        case 220:
        case 221:
        case 210:
            color = 'Yellow';
            break;

        case 111:
        case 222:
            color = 'White';
            break;

        default:
            color = 'UnkonwnColor';
    }
    $('#color').html('color : ' + color);
}