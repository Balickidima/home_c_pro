/**
 * @file root.h
 * @brief Заголовочный файл с объявлениями функций нахождения корней уравнений
 */

#ifndef ROOT_H
#define ROOT_H

#include "functions.h"

/**
 * @brief Структура для хранения результата поиска корня
 */
typedef struct {
    double koren;       /**< Найденный корень уравнения */
    int iteracii;       /**< Количество итераций */
    int shlo;           /**< Флаг сходимости (1 - сошлось, 0 - не сошлось) */
} RootResult;

/**
 * @brief Находит корень уравнения f(x) = g(x) на отрезке [levaya, pravaya] методом касательных (Ньютона)
 * @param f Первая функция
 * @param f_deriv Производная первой функции
 * @param g Вторая функция
 * @param g_deriv Производная второй функции
 * @param levaya Левая граница отрезка
 * @param pravaya Правая граница отрезка
 * @param tochnost1 Точность вычисления корня
 * @return Структура с найденным корнем и информацией о сходимости
 */
RootResult find_root(MathFunc f, MathFunc f_deriv, MathFunc g, MathFunc g_deriv, 
                    double levaya, double pravaya, double tochnost1);

/**
 * @brief Находит корень уравнения f(x) = 0 на отрезке [levaya, pravaya] методом касательных
 * @param f Функция
 * @param f_deriv Производная функции
 * @param levaya Левая граница отрезка
 * @param pravaya Правая граница отрезка
 * @param tochnost Точность вычисления
 * @return Структура с найденным корнем и информацией о сходимости
 */
RootResult find_root_odin(MathFunc f, MathFunc f_deriv, double levaya, double pravaya, double tochnost);

#endif /* ROOT_H */