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

void AddGaussianNoiseEns(std::vector<cv::Mat>& ensemble) {
  std::default_random_engine generator;
  int width = ensemble[0].cols;
  int height = ensemble[0].rows;

  for (int i = 0; i < height / 2; i += 1) {
    for (int j = 0; j < width; j += 1) {
      float sigma = std::abs(std::sin(i + j) * ((j + i) % 255));

      for (int k = 0; k < ensemble.size(); k += 1) {
        std::normal_distribution<float> distribution{ 0.0, sigma };
        float noise = std::abs(distribution(generator));
        ensemble[k].at<float>(cv::Point(i, j)) += noise;
      }
    }
  }

  for (int i = height / 2; i < height; i += 1) {
    for (int j = 0; j < width; j += 1) {
      float sigma = std::abs(std::sin(i + j) * ((j * i) % 255));

      for (int k = 0; k < ensemble.size(); k += 1) {
        std::normal_distribution<float> distribution{ 0.0, sigma };
        float noise = std::abs(distribution(generator));
        ensemble[k].at<float>(cv::Point(i, j)) += noise;
      }
    }
  }
}

std::vector<cv::Mat> CreateEnsemble(const cv::Mat& image, const int& amount) {
  std::vector<cv::Mat> noisy_images;

  float intense = 50.0;

  for (int i = 0; i < amount; i += 1) {
    cv::Mat noisy_image = cv::Mat::zeros(image.size(), CV_32FC1);
    image.copyTo(noisy_image);
    //noisy_image = AddRandomNoise(noisy_image, intense);
    noisy_images.push_back(noisy_image);
  }

  return noisy_images;
};

cv::Mat CircleMask(cv::Size size) {
  cv::Mat mask = cv::Mat::zeros(size, CV_8U);
  cv::Mat out_circle = cv::Mat::zeros(size, CV_8U);
  cv::Mat in_circle = cv::Mat::zeros(size, CV_8U);
  cv::circle(out_circle, cv::Point(mask.cols / 2, mask.rows / 2), 120, (255, 255, 255), cv::FILLED);
  cv::circle(in_circle, cv::Point(mask.cols / 2, mask.rows / 2), 40, (255, 255, 255), cv::FILLED);
  cv::subtract(out_circle, in_circle, mask);

  return mask;
}

int main(int argc, char* argv[]) {
  cv::Mat image = cv::imread("cat.png", cv::IMREAD_GRAYSCALE);
  cv::Mat image_32f = cv::Mat::zeros(image.size(), CV_32FC1);
  image.convertTo(image_32f, CV_32FC1);

  const int amount = 25;
  std::vector<cv::Mat> ensemble = std::move(CreateEnsemble(image_32f, amount));
  AddGaussianNoiseEns(ensemble);
  image_32f = ensemble[0];

  cv::Mat mask = CircleMask(image.size());
  cv::imwrite("mask.png", mask);
  
  NoiEst alg = NoiEst(image_32f, ensemble, amount, mask);
  
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