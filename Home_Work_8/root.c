/**
 * @file root.c
 * @brief Реализация функций нахождения корней уравнений методом касательных (Ньютона)
 */

#include "root.h"
#include <math.h>
#include <stdio.h>

#define MAX_ITERACII 1000

/**
 * Вспомогательная функция: разность двух funkc(x) - g(x)
 */
static double raznost_funkc(MathFunc f, MathFunc g, double x) {
    return f(x) - g(x);
}

/**
 * Вспомогательная функция: производная разности f'(x) - g'(x)
 */
static double raznost_proizvodnaya(MathFunc f_deriv, MathFunc g_deriv, double x) {
    return f_deriv(x) - g_deriv(x);
}

/**
 * Находит корень уравнения f(x) = g(x) на отрезке [levaya, pravaya] методом касательных (Ньютона)
 * Метод использует итерационную формулу: x_new = x_old - F(x_old) / F'(x_old),
 * где F(x) = f(x) - g(x)
 */
RootResult find_root(MathFunc f, MathFunc f_deriv, MathFunc g, MathFunc g_deriv, 
                    double levaya, double pravaya, double tochnost1) {
    RootResult result;
    double x, x_new;
    int iter = 0;
    
    /* Начальное приближение - середина отрезка */
    x = (levaya + pravaya) / 2.0;
    
    /* Проверка, что начальное приближение не слишком близко к производной, равной нулю */
    double deriv = raznost_proizvodnaya(f_deriv, g_deriv, x);
    if (fabs(deriv) < tochnost1) {
        /* Попробуем сдвинуться к концу отрезка */
        x = pravaya;
        deriv = raznost_proizvodnaya(f_deriv, g_deriv, x);
        if (fabs(deriv) < tochnost1) {
            x = levaya;
        }
    }
    
    /* Итерационный процесс метода Ньютона */
    for (iter = 0; iter < MAX_ITERACII; iter++) {
        double F = raznost_funkc(f, g, x);
        double F_deriv = raznost_proizvodnaya(f_deriv, g_deriv, x);
        
        /* Проверка на деление на ноль */
        if (fabs(F_deriv) < tochnost1) {
            result.koren = x;
            result.iteracii = iter;
            result.shlo = 0;
            return result;
        }
        
        /* Формула Ньютона */
        x_new = x - F / F_deriv;
        
        /* Проверка сходимости */
        if (fabs(x_new - x) < tochnost1) {
            result.koren = x_new;
            result.iteracii = iter + 1;
            result.shlo = 1;
            return result;
        }
        
        /* Проверка, что корень остается в разумных пределах */
        if (x_new < levaya - 10.0 || x_new > pravaya + 10.0) {
            result.koren = x;
            result.iteracii = iter;
            result.shlo = 0;
            return result;
        }
        
        x = x_new;
    }
    
    /* Достигнуто максимальное количество итераций */
    result.koren = x;
    result.iteracii = MAX_ITERACII;
    result.shlo = 0;
    return result;
}

/**
 * Находит корень уравнения f(x) = 0 на отрезке [levaya, pravaya] методом касательных
 */
RootResult find_root_odin(MathFunc f, MathFunc f_deriv, double levaya, double pravaya, double tochnost) {
    RootResult result;
    double x, x_new;
    int iter = 0;
    
    /* Начальное приближение */
    x = (levaya + pravaya) / 2.0;
    
    /* Проверка производной */
    double deriv = f_deriv(x);
    if (fabs(deriv) < tochnost) {
        x = pravaya;
        deriv = f_deriv(x);
        if (fabs(deriv) < tochnost) {
            x = levaya;
        }
    }
    
    /* Итерационный процесс */
    for (iter = 0; iter < MAX_ITERACII; iter++) {
        double F = f(x);
        double F_deriv = f_deriv(x);
        
        if (fabs(F_deriv) < tochnost) {
            result.koren = x;
            result.iteracii = iter;
            result.shlo = 0;
            return result;
        }
        
        x_new = x - F / F_deriv;
        
        if (fabs(x_new - x) < tochnost) {
            result.koren = x_new;
            result.iteracii = iter + 1;
            result.shlo = 1;
            return result;
        }
        
        if (x_new < levaya - 10.0 || x_new > pravaya + 10.0) {
            result.koren = x;
            result.iteracii = iter;
            result.shlo = 0;
            return result;
        }
        
        x = x_new;
    }
    
    result.koren = x;
    result.iteracii = MAX_ITERACII;
    result.shlo = 0;
    return result;
}