/**
 * @file functions.c
 * @brief Реализация математических функций lin_func, cub_func, inv_func и их производных
 */

#include "functions.h"

/**
 * Линейная функция lin_func(x) = 0.6*x + 3
 */
double lin_func(double argument) {
    return 0.6 * argument + 3.0;
}

/**
 * Производная линейной функции lin_deriv(x) = 0.6
 */
double lin_deriv(double argument) {
    (void)argument; /* Параметр не используется */
    return 0.6;
}

/**
 * Кубическая функция cub_func(x) = (x-2)^3 - 1
 */
double cub_func(double argument) {
    double t = argument - 2.0;
    return t * t * t - 1.0;
}

/**
 * Производная кубической функции cub_deriv(x) = 3*(x-2)^2
 */
double cub_deriv(double argument) {
    double t = argument - 2.0;
    return 3.0 * t * t;
}

/**
 * Обратная функция inv_func(x) = 3/x
 */
double inv_func(double argument) {
    return 3.0 / argument;
}

/**
 * Производная обратной функции inv_deriv(x) = -3/x^2
 */
double inv_deriv(double argument) {
    return -3.0 / (argument * argument);
}