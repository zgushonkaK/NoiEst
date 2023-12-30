#include "noi_est.hpp"

NoiEst::NoiEst(const cv::Mat image_32f, const std::vector<cv::Mat> ensemble, const int amount) {
  image_32f_ = image_32f;
  ensemble_ = ensemble;
  amount_ = amount;
}

NoiEst::NoiEst(const cv::Mat image_32f, const std::vector<cv::Mat> ensemble, const int amount, const cv::Mat& mask) {
  image_32f_ = image_32f;
  ensemble_ = ensemble;
  amount_ = amount;
  mask_ = mask;
}

NoiEst::~NoiEst() {
  ens_mean_.release();
  noise_map_.release();
  noise_hist_.release();
  noise_.release();
  roi_.release();
  n_low_.release();
  n_high_.release();
  m_low_.release();
  m_high_.release();
  rn_low_.release();
  rn_high_.release();
  rm_low_.release();
  rm_high_.release();
  r_low_.release();
  r_high_.release();
  nps_low_.release();
  nps_high_.release();
  nps_low_r_.release();
  nps_high_r_.release();
}

void NoiEst::EnsembleMean() {
  cv::Mat ensemble_sum = cv::Mat::zeros(image_32f_.size(), CV_32FC1);
  cv::Mat ensemble_mean = cv::Mat::zeros(image_32f_.size(), CV_32FC1);

  for (int i = 1; i < amount_; i += 1) {
    cv::Mat img_32f;
    ensemble_[i].convertTo(img_32f, CV_32FC1);
    ensemble_sum += img_32f;
  }

  ensemble_mean = ensemble_sum.clone();

  ensemble_mean /= (amount_ - 1);

  ens_mean_ = ensemble_mean;
}

cv::Mat NoiEst::mat2gray(const cv::Mat& image) {
  cv::Mat dst = image.clone();
  normalize(image, dst, 0.0, 1.0, cv::NORM_MINMAX);
  return dst;
}

void NoiEst::CreateNoiseMap() {
  cv::Mat sum = cv::Mat::zeros(ens_mean_.size(), ens_mean_.type());

  for (int i = 1; i < ensemble_.size(); i += 1) {
    cv::Mat square = cv::Mat::zeros(ens_mean_.size(), ens_mean_.type());
    cv::multiply((ensemble_[i] - ens_mean_), (ensemble_[i] - ens_mean_), square);
    sum += square;
  }

  cv::Mat divided = cv::Mat::zeros(ens_mean_.size(), ens_mean_.type());
  cv::divide(sum, ensemble_.size() - 2, divided);

  cv::Mat sigma;
  cv::sqrt(divided, sigma);

  noise_map_ = mat2gray(sigma);
}

void NoiEst::NoiseHist() {
  cv::Mat image_8bit;

  cv::normalize(noise_map_, image_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  int hist_size = 256;
  float range[] = { 0, 256};
  const float* hist_range = { range };
  bool uniform = true, accumulate = false;

  cv::Mat hist;
  cv::calcHist(&image_8bit, 1, 0, cv::Mat(), hist, 1, &hist_size, &hist_range, uniform, accumulate);

  int hist_w = 400;
  int hist_h = 255;
  int bin_w = cvRound((double)hist_w / hist_size);

  cv::Mat hist_img(hist_h, hist_w, CV_8UC1, cv::Scalar(0, 0, 0));

  normalize(hist, hist, 0, hist_img.rows, cv::NORM_MINMAX);

  for (int i = 1; i < hist_size; i += 1) {
    cv::line(hist_img, cv::Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
      cv::Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
      cv::Scalar(255, 0, 0), 2, 8, 0);
  }

  noise_hist_ = hist_img;
}

void NoiEst::ImageNoise() {
  cv::Mat noise = cv::Mat::zeros(image_32f_.size(), CV_8U);
  cv::Mat img_8bit, ens_8bit, noise_8bit;

  image_32f_.convertTo(img_8bit, CV_8U);
  ens_mean_.convertTo(ens_8bit, CV_8U);

  cv::subtract(img_8bit, ens_8bit, noise_8bit);
  noise_8bit.convertTo(noise, CV_32FC1);

  noise_ = image_32f_ - ens_mean_;
}

void NoiEst::SetRoiMask(const cv::Mat& mask) {
  mask_ = mask;
}

/*
cv::Mat NoiEst::OrigRoi(const cv::Mat& image_32f) {
  cv::Mat image;
  image_32f.convertTo(image, CV_8U);
  cv::Mat out_circle = cv::Mat::zeros(image.size(), image.type());
  cv::Mat in_circle = cv::Mat::zeros(image.size(), image.type());
  cv::Mat mask = cv::Mat::zeros(image.size(), image.type());

  cv::Mat roi_img_8bit = cv::Mat::zeros(image.size(), image.type());
  cv::Mat roi_img = cv::Mat::zeros(image_32f_.size(), image_32f_.type());

  double r_out = 120;
  double r_in = 40;

  cv::circle(out_circle, cv::Point(mask.cols / 2, mask.rows / 2), r_out, (255, 255, 255), cv::FILLED);
  cv::circle(in_circle, cv::Point(mask.cols / 2, mask.rows / 2), r_in, (255, 255, 255), cv::FILLED);

  cv::subtract(out_circle, in_circle, mask);

  image.copyTo(roi_img_8bit, mask);
  roi_img_8bit.convertTo(roi_img, CV_32FC1);

  return roi_img;
}
*/

cv::Mat NoiEst::Roi(const cv::Mat& image_32f) {
  cv::Mat image;
  cv::normalize(image_32f, image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::Mat roi_img = cv::Mat::zeros(image.size(), image.type());
  cv::bitwise_and(image, mask_, roi_img);
  roi_img.convertTo(roi_img, CV_32FC1);

  return roi_img;
}


cv::Mat NoiEst::Roi(const cv::Mat& image_32f, const cv::Mat& mask_32f) {
  cv::Mat image;
  cv::normalize(image_32f, image, 0, 255, cv::NORM_MINMAX, CV_8UC1);
  cv::Mat mask;
  cv::normalize(mask_32f, mask, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::Mat roi_img = cv::Mat::zeros(image.size(), image.type());
  cv::bitwise_and(image, mask, roi_img);
  roi_img.convertTo(roi_img, CV_32FC1);

  return roi_img;
}

void NoiEst::Roi() {
  roi_ = Roi(mat2gray(noise_));
}

void NoiEst::LowNoiseMask() {
  cv::Mat thresh_mask_low;
  cv::Mat mask_low;
  cv::threshold(noise_map_, thresh_mask_low, thresh_, max_val_, cv::THRESH_BINARY_INV);

  m_low_ = Roi(thresh_mask_low);
}

void NoiEst::HighNoiseMask() {
  cv::Mat thresh_mask_high;
  cv::threshold(noise_map_, thresh_mask_high, thresh_, max_val_, cv::THRESH_BINARY);

  m_high_ = Roi(thresh_mask_high);
}

void NoiEst::LowNoise() {
  cv::Mat thresh_mask_low;
  cv::threshold(noise_map_, thresh_mask_low, thresh_, max_val_, cv::THRESH_BINARY_INV);

  cv::Mat n_low = cv::Mat::zeros(image_32f_.size(), CV_32FC1);
  cv::multiply(roi_, thresh_mask_low, n_low);

  n_low_ = n_low;
}

void NoiEst::HighNoise() {
  cv::Mat thresh_mask_high;
  cv::threshold(noise_map_, thresh_mask_high, thresh_, max_val_, cv::THRESH_BINARY);

  cv::Mat n_high = cv::Mat::zeros(image_32f_.size(), CV_32FC1);
  cv::multiply(roi_, thresh_mask_high, n_high);

  n_high_ = n_high;
}

const cv::Mat NoiEst::Correlation(const cv::Mat& image1, const cv::Mat& image2) {
  CV_Assert(image1.size() == image2.size());

  cv::Mat result(image1.size(), CV_32FC1, cv::Scalar(0));

  cv::Mat kernel;
  cv::flip(image2, kernel, -1);

  cv::filter2D(image1, result, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);

  return result;
}

void NoiEst::LowNoiseCorrelation() {
  rn_low_ = Correlation(Roi(n_low_, m_low_), Roi(n_low_, m_low_));
}

void NoiEst::HighNoiseCorrelation() {
  rn_high_ = Correlation(Roi(n_high_, m_high_), Roi(n_high_, m_high_));
}

void NoiEst::LowMaskCorrelation() {
  rm_low_ = Correlation(m_low_, m_low_);
}

void NoiEst::HighMaskCorrelation() {
  rm_high_ = Correlation(m_high_, n_high_);
}

const cv::Mat NoiEst::AutoCorrelation(const cv::Mat& noise, const cv::Mat& mask) {
  cv::Mat scaled_rn;
  cv::normalize(noise, scaled_rn, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::Mat scaled_rm;
  cv::normalize(mask, scaled_rm, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::Mat autocorr = scaled_rn;
  autocorr /= scaled_rm;

  return autocorr;
}

void NoiEst::LowCorrelation() {
  r_low_ = AutoCorrelation(rn_low_, rm_low_);
}

void NoiEst::HighCorrelation() {
  r_high_ = AutoCorrelation(rn_high_, rm_high_);
}

const cv::Mat NoiEst::Dft(const cv::Mat& image) {
  cv::Size dft_size;
  cv::Mat image_32f;
  image.convertTo(image_32f, CV_32FC1);

  dft_size.width = cv::getOptimalDFTSize(image_32f.cols);
  dft_size.height = cv::getOptimalDFTSize(image_32f.rows);

  cv::Mat padded;
  copyMakeBorder(image_32f, padded, 0, dft_size.height - image_32f.rows, 0, dft_size.width - image_32f.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

  cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32FC1) };
  cv::Mat complex_img;
  cv::merge(planes, 2, complex_img);

  cv::dft(complex_img, complex_img);
  cv::split(complex_img, planes);
  cv::magnitude(planes[0], planes[1], planes[0]);
  cv::Mat mag_img = planes[0];

  mag_img += cv::Scalar::all(1);
  cv::log(mag_img, mag_img);

  mag_img = mag_img(cv::Rect(0, 0, mag_img.cols & -2, mag_img.rows & -2));

  int cx = mag_img.cols / 2;
  int cy = mag_img.rows / 2;
  cv::Mat q0(mag_img, cv::Rect(0, 0, cx, cy));
  cv::Mat q1(mag_img, cv::Rect(cx, 0, cx, cy));
  cv::Mat q2(mag_img, cv::Rect(0, cy, cx, cy));
  cv::Mat q3(mag_img, cv::Rect(cx, cy, cx, cy));

  cv::Mat tmp;
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp);
  q2.copyTo(q1);
  tmp.copyTo(q2);

  cv::Mat res = cv::Mat::zeros(image_32f.size(), image_32f.type());
  cv::normalize(mag_img, mag_img, 0, 1, cv::NORM_MINMAX);
  cv::multiply(image_32f, mag_img, res);
  return res;
}

void NoiEst::LowDft() {
  nps_low_ = Dft(r_low_);
}

void NoiEst::HighDft() {
  nps_high_ = Dft(r_high_);
}

const cv::Mat NoiEst::RadialPlot(const cv::Mat& image) {
  cv::Mat nps_hist = cv::Mat::zeros(1, image.cols, CV_32FC1);
  std::vector<int> cnt(image.cols, 0);
  int width = image.cols;
  int height = image.rows;

  int x_center = width / 2;
  int y_center = height / 2;

  cv::Mat nps = nps_hist;

  for (int x = 0; x < height; x += 1) {
    for (int y = 0; y < width; y += 1) {
      int r = std::sqrt(std::abs(x_center - x) * std::abs(x_center - x) + std::abs(y_center - y) * std::abs(y_center - y));
      float nps_val = image.at<float>(cv::Point(x, y));
      nps.at<float>(r) += nps_val;
    }
  }

  int hist_w = 400;
  int hist_h = 255;
  int hist_size = image.cols / 2;
  int bin_w = cvRound((float)hist_w / hist_size);

  normalize(nps, nps, 0, 255, cv::NORM_MINMAX);
  cv::Mat nps_hist_img(hist_h, hist_w, CV_8UC1, cv::Scalar(0, 0, 0));

  for (int i = 1; i < hist_size; i += 1) {
    cv::line(nps_hist_img, cv::Point(bin_w * (i - 1), hist_h - cvRound(nps.at<float>(i - 1))),
      cv::Point(bin_w * (i), hist_h - cvRound(nps.at<float>(i))),
      cv::Scalar(255, 0, 0), 2, 8, 0);
  }

  return nps_hist_img;
}

void NoiEst::LowNps() {
  nps_low_r_ = RadialPlot(nps_low_);
}

void NoiEst::HighNps() {
  nps_high_r_ = RadialPlot(nps_high_);
}

const void NoiEst::Calculate() {
  EnsembleMean();
  CreateNoiseMap();
  ImageNoise();
  Roi();
  NoiseHist();
  LowNoiseMask();
  HighNoiseMask();
  LowNoise();
  HighNoise();

  LowNoiseCorrelation();
  LowMaskCorrelation();
  HighNoiseCorrelation();
  HighMaskCorrelation();

  LowCorrelation();
  HighCorrelation();

  LowDft();
  HighDft();

  LowNps();
  HighNps();
}

const void NoiEst::Show(const cv::Mat& image, const std::string& name) {
  cv::Mat scaled_image;
  cv::normalize(image, scaled_image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::Mat output_image;
  scaled_image.convertTo(output_image, CV_8UC1);

  cv::imshow(name, output_image);
}

const void NoiEst::Save(const cv::Mat& image, const std::string& name) {
  cv::Mat scaled_image;
  cv::normalize(image, scaled_image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::Mat output_image;
  scaled_image.convertTo(output_image, CV_8UC1);

  std::string file_name = name + ".png";

  cv::imwrite(file_name, output_image);
}

const void NoiEst::DefaultSave() {
  Save(image_32f_, "source_img");
  Save(noise_hist_, "noise_hist");
  Save(noise_map_, "noise_map");
  Save(ens_mean_, "ens_mean");
  Save(noise_, "noise");
  Save(roi_, "roi");
  Save(n_low_, "n'_low");
  Save(n_high_, "n'_high");
  Save(rn_low_, "rn_low");
  Save(rn_high_, "rn_high");
  Save(m_low_, "m_low");
  Save(m_high_, "m_high");
  Save(rm_low_, "rm_low");
  Save(rm_high_, "rm_high");
  Save(r_low_, "r_low");
  Save(r_high_, "r_high");
  Save(nps_low_, "nps_low");
  Save(nps_high_, "nps_high");
  Save(nps_low_r_, "nps_low_r");
  Save(nps_high_r_, "nps_high_r");
}

void NoiEst::SetThresh(float thresh){thresh_ = std::move(thresh);}