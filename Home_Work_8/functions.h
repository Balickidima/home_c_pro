/**
 * @file functions.h
 * @brief Заголовочный файл с объявлениями математических функций
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <math.h>

/**
 * @brief Линейная функция lin_func(x) = 0.6*x + 3
 * @param argument аргумент функции
 * @return значение функции в точке argument
 */
double lin_func(double argument);

/**
 * @brief Производная линейной функции lin_deriv(x) = 0.6
 * @param argument аргумент функции (не используется)
 * @return значение производной
 */
double lin_deriv(double argument);

/**
 * @brief Кубическая функция cub_func(x) = (x-2)^3 - 1
 * @param argument аргумент функции
 * @return значение функции в точке argument
 */
double cub_func(double argument);

/**
 * @brief Производная кубической функции cub_deriv(x) = 3*(x-2)^2
 * @param argument аргумент функции
 * @return значение производной
 */
double cub_deriv(double argument);

/**
 * @brief Обратная функция inv_func(x) = 3/x
 * @param argument аргумент функции
 * @return значение функции в точке argument
 */
double inv_func(double argument);

/**
 * @brief Производная обратной функции inv_deriv(x) = -3/x^2
 * @param argument аргумент функции
 * @return значение производной
 */
double inv_deriv(double argument);

/**
 * @brief Указатель на функцию одной переменной
 */
typedef double (*MathFunc)(double);

#endif /* FUNCTIONS_H */