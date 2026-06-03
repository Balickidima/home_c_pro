/**
 * @file integral.c
 * @brief Реализация функции вычисления определённого интеграла методом Симпсона
 */

#include "integral.h"
#include <math.h>

#define MAX_ITERACII 100

/**
 * Вычисляет интеграл методом Симпсона с заданным числом разбиений
 * Формула Симпсона: I ≈ h/3 * [f(x0) + 4*f(x1) + 2*f(x2) + 4*f(x3) + ... + f(xn)]
 * где n должно быть чётным
 */
static double metod_simpsona(MathFunc f, double levaya, double pravaya, int n) {
    double shag = (pravaya - levaya) / n;
    double summa = f(levaya) + f(pravaya);
    
    for (int i = 1; i < n; i++) {
        double x = levaya + i * shag;
        if (i % 2 == 0) {
            summa += 2.0 * f(x);
        } else {
            summa += 4.0 * f(x);
        }
    }
    
    return shag * summa / 3.0;
}

/**
 * Вычисляет определённый интеграл от функции f(x) на отрезке [levaya, pravaya] методом Симпсона
 * с адаптивным выбором числа разбиений для достижения заданной точности
 */
double calc_integral(MathFunc f, double levaya, double pravaya, double tochnost2) {
    int n = 2; /* Начальное число разбиений (должно быть чётным) */
    double pred_rezultat, tek_rezultat;
    
    /* Если levaya > pravaya, меняем местами */
    if (levaya > pravaya) {
        double temp = levaya;
        levaya = pravaya;
        pravaya = temp;
    }
    
    /* Начальное вычисление */
    pred_rezultat = metod_simpsona(f, levaya, pravaya, n);
    
    /* Увеличиваем число разбиений до достижения точности */
    for (int iter = 0; iter < MAX_ITERACII; iter++) {
        n *= 2; /* Удваиваем число разбиений */
        tek_rezultat = metod_simpsona(f, levaya, pravaya, n);
        
        /* Проверка точности */
        if (fabs(tek_rezultat - pred_rezultat) < tochnost2) {
            return tek_rezultat;
        }
        
        pred_rezultat = tek_rezultat;
    }
    
    /* Возвращаем результат даже если точность не достигнута */
    return tek_rezultat;
}