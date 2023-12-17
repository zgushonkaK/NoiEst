#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "noi_est.hpp"
/*
const void Save(const cv::Mat& image, const std::string& name) {
  cv::Mat scaled_image;
  cv::normalize(image, scaled_image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::Mat output_image;
  scaled_image.convertTo(output_image, CV_8UC1);

  std::string file_name = name + ".png";

  cv::imwrite(file_name, output_image);
}
*/

cv::Mat AddRandomNoise(cv::Mat image, float noise_intensity) {
  cv::Mat noise(image.size(), image.type());
  cv::randn(noise, noise_intensity, 2 * noise_intensity);

  image += noise;
  return image;
}

/*
cv::Mat AddGaussianNoise(cv::Mat& image) {
  std::default_random_engine generator;
  cv::Mat sigma_map(image.size(), image.type());
  for (int i = 0; i < image.rows; i += 1) {
    for (int j = 0; j < image.cols; j += 1) {
      float sigma = std::abs(std::sin((i + j)*0.005));

      //sigma = 0.1 + (sigma + 1.0) * 0.3;
      //sigma *= 0.01;
      std::normal_distribution<float> distribution{ 0.0, sigma };

      float noise = distribution(generator);

      float pixel = image.at<float>(i, j);
      image.at<float>(i, j) += noise;
      //sigma_map.at<float>(i, j) = sigma;
    }
  }
  
  //Save(sigma_map, "sigma");
  return image;
}*/

void AddGaussianNoiseEns(std::vector<cv::Mat>& ensemble) {
  std::default_random_engine generator;
  int width = ensemble[0].cols;
  int height = ensemble[0].rows;

  for (int i = 0; i < height; i += 1) {
    for (int j = 0; j < width; j += 1) {
      float sigma = std::abs(std::sin(i + j)*i);

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
    //noisy_image = AddGaussianNoise(noisy_image);
    noisy_images.push_back(noisy_image);
  }

  return noisy_images;
};

int main(int argc, char* argv[]) {
  cv::Mat image = cv::imread("grid.png", cv::IMREAD_GRAYSCALE);
  cv::Mat image_32f = cv::Mat::zeros(image.size(), CV_32FC1);
  image.convertTo(image_32f, CV_32FC1);

  const int amount = 25;
  std::vector<cv::Mat> ensemble = std::move(CreateEnsemble(image_32f, amount));
  AddGaussianNoiseEns(ensemble);
  image_32f = ensemble[0];
  //GridImage(image_32f);

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