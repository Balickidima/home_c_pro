/**
 * @file integral.h
 * @brief Заголовочный файл с объявлениями функций вычисления определённых интегралов
 */

#ifndef INTEGRAL_H
#define INTEGRAL_H

#include "functions.h"

/**
 * @brief Вычисляет определённый интеграл от функции f(x) на отрезке [levaya, pravaya] методом Симпсона
 * @param f Подынтегральная функция
 * @param levaya Левая граница отрезка
 * @param pravaya Правая граница отрезка
 * @param tochnost2 Точность вычисления интеграла
 * @return Значение определённого интеграла
 */
double calc_integral(MathFunc f, double levaya, double pravaya, double tochnost2);

#endif /* INTEGRAL_H */