const applyRandomNoise = (pixels, variance = 10) => {
  for (let i = 0; i < pixels.length; i++) {
    const noise = Math.round(Math.random() * variance * 2 - variance);
    pixels[i] += noise;
  }
};

const applySaltAndPepperNoise = (
  pixels,
  saltProbability = 0.01,
  pepperProbability = 0.01
) => {
  for (let i = 0; i < pixels.length; i++) {
    if (Math.random() < saltProbability) {
      pixels[i] = 127;
    } else if (Math.random() < pepperProbability) {
      pixels[i] = -128;
    }
  }
};

module.exports = { applyRandomNoise, applySaltAndPepperNoise };
