#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "noi_est.hpp"

cv::Mat AddRandomNoise(cv::Mat image, float noise_intensity) {
  cv::Mat noise(image.size(), image.type());
  cv::randn(noise, noise_intensity, 2 * noise_intensity);

  image += noise;
  return image;
}

std::vector<cv::Mat> CreateEnsemble(const cv::Mat& image, const int& amount) {
  std::vector<cv::Mat> noisy_images;

  float intense = 50.0;

  for (int i = 0; i < amount; i += 1) {
    cv::Mat noisy_image = cv::Mat::zeros(image.size(), CV_32FC1);
    image.copyTo(noisy_image);

    noisy_image = AddRandomNoise(noisy_image, intense);
    noisy_images.push_back(noisy_image);
  }

  return noisy_images;
};

int main() {
  cv::Mat image = cv::imread("cat.png", cv::IMREAD_GRAYSCALE);
  cv::Mat image_32f = cv::Mat::zeros(image.size(), CV_32FC1);
  image.convertTo(image_32f, CV_32FC1);

  const int amount = 21;
  std::vector<cv::Mat> ensemble = std::move(CreateEnsemble(image_32f, amount));
  image_32f = ensemble[0];

  NoiEst alg = NoiEst(image_32f, ensemble, amount);
  alg.Calculate();
  alg.DefaultSave();
  return 0;
}