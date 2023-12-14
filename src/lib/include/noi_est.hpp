#ifndef _NOI_EST_HPP_
#define _NOI_EST_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

/**
* @brief Класс для вычисления этапов алгоритма
*/

class NoiEst{
private:
  //! Исходная картинка
  cv::Mat image_32f_;
  //! Количество изображений в ансамбле
  int amount_;
  //! Ансамбль
  std::vector<cv::Mat> ensemble_;

  //! Порог
  float thresh_ = 0.5;
  //! Присваиваемое максимальное значение при разделении по порогу
  float max_val_ = 1.0;

  //! Среднее изображение по ансамблю
  cv::Mat ens_mean_;
  //! Карта шумов
  cv::Mat noise_map_;
  //! Гистограмма величины шумов
  cv::Mat noise_hist_;
  //! Изображение с нулевым средним шумом
  cv::Mat noise_;
  //! Маска области интереса
  cv::Mat roi_;
  //! Изображение с низким шумом
  cv::Mat n_low_;
  //! Изображение с высоким шумом
  cv::Mat n_high_;
  //! Бинарная маска для области с низким шумом
  cv::Mat m_low_;
  //! Бинарная маска для области с высоким шумом
  cv::Mat m_high_;
  //! Результат корреляции для изображения с низким шумом
  cv::Mat rn_low_;
  //! Результат корреляции для изображения с высоким шумом
  cv::Mat rn_high_;
  //! Результат корреляции для бинарной маски с низким шумом
  cv::Mat rm_low_;
  //! Результат корреляции для бинарной маски с высоким шумом
  cv::Mat rm_high_;
  //! Результат автокорреляции для низкого шума
  cv::Mat r_low_;
  //! Результат автокорреляции для высокого шума
  cv::Mat r_high_;
  //! Спектр мощности низкого шума
  cv::Mat nps_low_;
  //! Спектр мощности выского шума
  cv::Mat nps_high_;
  //! Кривая для спектра низкого шума
  cv::Mat nps_low_r_;
  //! Кривая для спектра выского шума
  cv::Mat nps_high_r_;

public:
  //! Конструктор
  NoiEst(cv::Mat image_32f, std::vector<cv::Mat> ensemble, int amount);

  //! Деструктор
  ~NoiEst();

  /*!
  * @brief Нормализирует изображение
  * @param image Изображение, которое необходимо нормализовать
  */
  cv::Mat mat2gray(const cv::Mat& image);
  
  /*!
  * @brief Высчитывает среднее по ансамблю
  */
  void EnsembleMean();
 
  /*!
  * @brief Создаёт карту шумов
  */
  void CreateNoiseMap();
  
  /*!
  * @brief Создаёт изображение с нулевым средним шумом
  */
  void ImageNoise();
 
  //! @private
  cv::Mat RoundRoi(const cv::Mat& image_32f);
  
  /*!
  * @brief Выделяет область интереса
  * исходного изображения
  * @details Применяет к изображению маску:
  * круг с внешним радиусом 120 пикселей и внутренним 40
  */
  void OrigRoi();
  
  /*!
  * @brief Создаёт гистограмму шума
  */
  void NoiseHist();
  
  /*!
  * @brief Создаёт бинарную маску для области с низким шумом
  */
  void LowNoiseMask();

  /*!
  * @brief Создаёт бинарную маску для области с высоким шумом
  */
  void HighNoiseMask();

  /*!
  * @brief Получает изображение с низким шумом в области интереса
  */
  void LowNoise();

  /*!
  * @brief Получает изображение с высоким шумом в области интереса
  */
  void HighNoise();

  //! @private
  const cv::Mat Correlation(const cv::Mat& image1, const cv::Mat& image2);

  /*!
  * @brief Вычисляет корреляцию области с низким шумом
  */
  void LowNoiseCorrelation();

  /*!
  * @brief Вычисляет корреляцию бинарной маски с низким шумом
  */
  void LowMaskCorrelation();

  /*!
  * @brief Вычисляет корреляцию области с высоким шумом
  */
  void HighNoiseCorrelation();

  /*!
  * @brief Вычисляет корреляцию бинарной маски с
  * высоким шумом
  */
  void HighMaskCorrelation();

  //! @private
  const cv::Mat AutoCorrelation(const cv::Mat& noise, const cv::Mat& mask);

  /*!
  * @brief Вычисляет автокорреляцию изображения с
  * низким шумом
  */
  void LowCorrelation();

  /*!
  * @brief Вычисляет автокорреляцию изображения с
  * высоким шумом
  */
  void HighCorrelation();

  //! @private
  const cv::Mat Dft(const cv::Mat& image_32f);

  /*!
  * @brief Вычисляет величину дискретного 
  * преобразования Фурье для области с 
  * низким шумом
  */
  void LowDft();

  /*!
  * @brief Вычисляет величину дискретного
  * преобразования Фурье для области с 
  * высоким шумом
  */
  void HighDft();

  //! @private
  const cv::Mat RadialPlot(const cv::Mat& image);

  /*!
  * @brief Строит кривую спектра низкого шума
  * с привязкой к расстоянию от центра
  */
  void LowNps();

  /*!
  * @brief Строит кривую спектра высокого шума
  * с привязкой к расстоянию от центра
  */
  void HighNps();

  /*!
  * @brief Выводит изображение на экран
  * @param[in] image Изображение 
  * @param[in] name Название окна
  */
  const void Show(const cv::Mat& image, const std::string& name);

  /*!
  * @brief Сохраняет изображение в файл
  * @param[in] image Изображение
  * @param[in] name Название файла
  */
  const void Save(const cv::Mat& image, const std::string& name);

  /*!
  * @brief Вызывает все методы для получения
  * конечного результата
  */
  const void Calculate();

  /*!
  * @brief Сохраняет все поля экземпляра в файлы
  */
  const void DefaultSave();

  /*!
  * @brief Позволяет установить порог
  */
  void SetThresh(float thresh);
};

#endif //_NOI_EST_HPP_