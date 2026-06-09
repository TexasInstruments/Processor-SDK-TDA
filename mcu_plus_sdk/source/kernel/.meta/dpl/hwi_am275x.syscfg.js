
let common = system.getScript("/common");

let intcBaseAddr_r5f = 0x2fff0000;

function getIntcBaseAddr() {

    return intcBaseAddr_r5f;
}

exports = {
    getIntcBaseAddr,
};

