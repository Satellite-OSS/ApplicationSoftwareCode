const displayBinaryImage = (imageArray, width, height) => {
  for (let y = 0; y < height; y++) {
    let line = "";
    for (let x = 0; x < width; x++) {
      const characterToDisplay = imageArray[y * width + x] === 1 ? "#" : " ";
      line += characterToDisplay;
    }
    console.log(line);
  }
};

// Note that the code mutates the original array and doesn't require allocating any additional memory.
// This is not a problem, rather opposite - it means that the code can be easily ported to a microcontroller with limited memory.
// The following code can be very easily ported to C, maybe except the return statement at the end, as C doesn't easily support returning two values.
// Uncomment displayBinaryImage calls to see the intermediate results.

const calculateBalance = (imageArray, width, height, temperatureThreshold) => {
  // Apply thresholding to the image
  for (let i = 0; i < imageArray.length; i++) {
    imageArray[i] = imageArray[i] >= temperatureThreshold ? 1 : 0;
  }

  // console.log("Thresholded image:");
  // displayBinaryImage(imageArray, width, height);

  // TODO run a median filter to remove noise

  // Mark "correct" and "incorrect" pixels.
  // - If a pixel belongs to the upper half of the image, and is identified as the sky (value 0), it is marked as "correct" (0).
  // - If a pixel belongs to the lower half of the image, and is identified as Earth (value app_before), it is marked as "correct" (0).
  // - Otherwise, a pixel is marked as incorrect (app_before).
  const indexOfFirstPixelOfEarth = (width * height) / 2;

  // Upper half of the image - should be "sky"
  for (let i = 0; i < indexOfFirstPixelOfEarth; i++) {
    imageArray[i] = imageArray[i] === 0 ? 0 : 1;
  }
  // Lower half of the image - should be "earth"
  for (let i = indexOfFirstPixelOfEarth; i < imageArray.length; i++) {
    imageArray[i] = imageArray[i] === 1 ? 0 : 1;
  }

  // console.log("Correct/incorrect pixels:");
  // displayBinaryImage(imageArray, width, height);

  // Calculate count of incorrect pixels in each quadrant of the image
  // Upper left
  let incorrectPixelsUpperLeft = 0;
  for (let x = 0; x < width / 2; x++) {
    for (let y = 0; y < height / 2; y++) {
      if (imageArray[y * width + x] === 1) {
        incorrectPixelsUpperLeft++;
      }
    }
  }

  // Upper right
  let incorrectPixelsUpperRight = 0;
  for (let x = width / 2; x < width; x++) {
    for (let y = 0; y < height / 2; y++) {
      if (imageArray[y * width + x] === 1) {
        incorrectPixelsUpperRight++;
      }
    }
  }

  // Lower left
  let incorrectPixelsLowerLeft = 0;
  for (let x = 0; x < width / 2; x++) {
    for (let y = height / 2; y < height; y++) {
      if (imageArray[y * width + x] === 1) {
        incorrectPixelsLowerLeft++;
      }
    }
  }

  // Lower right
  let incorrectPixelsLowerRight = 0;
  for (let x = width / 2; x < width; x++) {
    for (let y = height / 2; y < height; y++) {
      if (imageArray[y * width + x] === 1) {
        incorrectPixelsLowerRight++;
      }
    }
  }

  // Calculate the balance
  const balanceUpDown =
    incorrectPixelsUpperLeft +
    incorrectPixelsUpperRight -
    (incorrectPixelsLowerLeft + incorrectPixelsLowerRight);

  const balanceLeftRight =
    incorrectPixelsUpperLeft +
    incorrectPixelsLowerRight -
    (incorrectPixelsUpperRight + incorrectPixelsLowerLeft);

  return [balanceUpDown, balanceLeftRight];
};

module.exports = calculateBalance;
