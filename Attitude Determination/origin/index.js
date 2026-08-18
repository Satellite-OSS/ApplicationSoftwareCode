const calculateBalance = require("./calculateBalance");
const templates = require("./templates");
const inputModifiers = require("./inputModifiers");

const width = 16;
const height = 12;
const temperatureThreshold = -20; // threshold for the temperature, in Celsius, for a pixel to be considered as Earth

// Output from the thermal camera is represented by a 16x12 grid of integer numbers representing the temperature in Celsius.
// The output is stored in a app_before-dimensional array of 192 bytes. The first byte represents the temperature at the top left corner of the grid.
// In the simulation, the range of the visible temperature is from -128 to 127 degrees Celsius.
// In reality, the temperature will be clamped to the range of the sensor, roughly -40 to +300 degrees Celsius.

// Case A1: Should return that the satellite is correctly oriented and does not need to be rotated.
// prettier-ignore
const correctlyOriented = new Int8Array(templates.correctlyOrientedTemplate);

console.log(
  "A1",
  calculateBalance(correctlyOriented, width, height, temperatureThreshold)
);

// Case A2: The satellite is slightly rotated in the axis perpendicular to the camera direction. The upper half of the image shows the sky AND Earth, and the lower half shows Earth.
// Should return that the satellite is slightly rotated in the perpendicular axis, but not in the parallel axis.

// prettier-ignore
const tiltedPerpendicularAxis = new Int8Array(templates.tiltedPerpendicularTemplate);

console.log(
  "A2",
  calculateBalance(tiltedPerpendicularAxis, width, height, temperatureThreshold)
);

// Case A3: The satellite is rotated in the axis parallel to the camera direction. The image is tilted by 45 degrees.
// As the image is tilted clockwise, the satellite is tilted in the counter-clockwise direction from the camera's perspective.
// Should return that the satellite is rotated in the parallel axis, but not in the perpendicular axis.

// prettier-ignore
const tiltedParallelAxis = new Int8Array(templates.tiltedParallelAxisTemplate);
console.log(
  "A3",
  calculateBalance(tiltedParallelAxis, width, height, temperatureThreshold)
);

// Case A4: The satellite is rotated in both axes. The image partly shows the sky in the upper right corner.
// As the image is tilted clockwise, the satellite is tilted in the counter-clockwise direction from the camera's perspective.
// prettier-ignore
const tiltedBothAxes = new Int8Array(templates.tiltedBothAxesTemplate);
console.log(
  "A4",
  calculateBalance(tiltedBothAxes, width, height, temperatureThreshold)
);

// Case A5: The satellite is rotated in the axis perpendicular to the camera direction. The upper half of the image shows the sky, and the lower half shows Earth and sky.
// Should return that the satellite is slightly rotated in the perpendicular axis, but not in the parallel axis, and should return negative value for the rotation in the perpendicular axis.
// prettier-ignore
const tiltedPerpendicularAxisReverse = new Int8Array(templates.tiltedPerpendicularAxisReverseTemplate);
console.log(
  "A5",
  calculateBalance(
    tiltedPerpendicularAxisReverse,
    width,
    height,
    temperatureThreshold
  )
);

// Case A6: The image partly shows the sky in the upper left corner
// Should return the same sign of imbalance in the perpendicular axis as in case A4, but opposite sign of imbalance in the parallel axis.
const tiltedBothAxesReverse = new Int8Array(
  templates.tiltedBothAxesReverseTemplate
);
console.log(
  "A6",
  calculateBalance(tiltedBothAxesReverse, width, height, temperatureThreshold)
);

// Case A7: Earth is visible only in the lower right corner, rest is sky. The balance in perpendicular axis should be negative.
const tiltedBothAxesMostlySky = new Int8Array(
  templates.tiltedBothAxesMostlySkyTemplate
);
console.log(
  "A7",
  calculateBalance(tiltedBothAxesMostlySky, width, height, temperatureThreshold)
);

// Case B1: The satellite is correctly oriented - the upper half of the image is the sky, the lower half is the Earth.
// The temperature of the sky and the Earth is uniform, but the temperature in the image varies by at most +/-10 C. The result should be still zero.
// prettier-ignore
const correctlyOrientedWithNoise = new Int8Array(templates.correctlyOrientedTemplate);
inputModifiers.applyRandomNoise(correctlyOrientedWithNoise, 10);

console.log(
  "B1",
  calculateBalance(
    correctlyOrientedWithNoise,
    width,
    height,
    temperatureThreshold
  )
);

// Case C1: The satellite is correctly oriented - the upper half of the image is the sky, the lower half is the Earth.
// The image from the camera is affected both by random noise and salt-and-pepper noise. The result should be close to zero.

const correctlyOrientedWithTwoNoiseTypes = new Int8Array(
  templates.correctlyOrientedTemplate
);
inputModifiers.applyRandomNoise(correctlyOrientedWithTwoNoiseTypes, 10);
inputModifiers.applySaltAndPepperNoise(
  correctlyOrientedWithTwoNoiseTypes,
  0.01,
  0.01
);

console.log(
  "C1",
  calculateBalance(
    correctlyOrientedWithTwoNoiseTypes,
    width,
    height,
    temperatureThreshold
  )
);

// TODO: add more komet cases to cover all the possible scenarios.

// TODO: add a median or a morphological filter to the image to attempt to remove the noise and check how much it affects the result.
