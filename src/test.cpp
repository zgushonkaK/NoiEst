#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "noi_est.hpp"

cv::Mat AddRandomNoise(cv::Mat image, float noise_intensity) {
  cv::Mat noise(image.size(), image.type());
  cv::randn(noise, noise_intensity, 2 * noise_intensity);

  image += noise;
  return image;
}

cv::Mat AddGaussianNoise(cv::Mat& image) {
  std::default_random_engine generator;

  for (int i = 0; i < image.rows; i += 1) {
    for (int j = 0; j < image.cols; j += 1) {
      float sigma = std::abs(std::sin(i + j));

      //sigma = 0.1 + (sigma + 1.0) * 0.3;
      //sigma *= 0.01;
      std::normal_distribution<float> distribution{ 0.0, sigma };

      float noise = std::abs(distribution(generator));

      float pixel = image.at<float>(i, j);
      image.at<float>(i, j) += noise;
    }
  }
  return image;
}

std::vector<cv::Mat> CreateEnsemble(const cv::Mat& image, const int& amount) {
  std::vector<cv::Mat> noisy_images;

  float intense = 50.0;

  for (int i = 0; i < amount; i += 1) {
    cv::Mat noisy_image = cv::Mat::zeros(image.size(), CV_32FC1);
    image.copyTo(noisy_image);

    //noisy_image = AddRandomNoise(noisy_image, intense);
    noisy_image = AddGaussianNoise(noisy_image);
    noisy_images.push_back(noisy_image);
  }

  return noisy_images;
};

int main(int argc, char* argv[]) {
  cv::Mat image = cv::imread("cat.png", cv::IMREAD_GRAYSCALE);
  cv::Mat image_32f = cv::Mat::zeros(image.size(), CV_32FC1);
  image.convertTo(image_32f, CV_32FC1);

  const int amount = 25;
  std::vector<cv::Mat> ensemble = std::move(CreateEnsemble(image_32f, amount));
  image_32f = ensemble[0];

  NoiEst alg = NoiEst(image_32f, ensemble, amount);
  
  /*
  if (argc == 2) {
    float threshold = std::atof(argv[1]);
    if (threshold > 0 && threshold < 1.0) {
      alg.SetThresh(threshold);
    }
  }*/

  alg.Calculate();
  alg.DefaultSave();
  return 0;
}