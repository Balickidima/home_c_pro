/**
 * @file main.c
 * @brief Основная программа для вычисления площади, образуемой функциями
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "functions.h"
#include "root.h"
#include "integral.h"

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Точность вычислений
 */
#define TOCHNOST1 1e-6
#define TOCHNOST2 1e-6

/**
 * @brief Структура для хранения точки пересечения
 */
typedef struct {
    double x;          /**< Абсцисса точки пересечения */
    int iteracii;      /**< Количество итераций для нахождения точки */
} TochkaPeresecheniya;

/**
 * @brief Находит точки пересечения между функциями
 * @param tochki Массив для хранения точек пересечения
 * @param max_tochek Максимальное количество точек
 * @return Количество найденных точек
 */
int najti_peresecheniya(TochkaPeresecheniya tochki[], int max_tochek);

/**
 * @brief Вычисляет площадь, образуемую функциями
 * @return Вычисленная площадь
 */
double vychislit_ploshad();

/**
 * @brief Тестирует функции find_root и calc_integral
 */
void testirovat_funkcii();

/**
 * @brief Выводит справку по использованию программы
 */
void vyvesti_spravku();

/**
 * @brief Главная функция программы
 */
int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    int pechatat_peresecheniya = 0;
    int pechatat_iteracii = 0;
    int zapusk_testov = 0;
    
    /* Парсинг аргументов командной строки */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-help") == 0) {
            vyvesti_spravku();
            return 0;
        } else if (strcmp(argv[i], "-intersections") == 0) {
            pechatat_peresecheniya = 1;
        } else if (strcmp(argv[i], "-iterations") == 0) {
            pechatat_iteracii = 1;
        } else if (strcmp(argv[i], "-test") == 0) {
            zapusk_testov = 1;
        } else {
            fprintf(stderr, "Неизвестный ключ: %s\n", argv[i]);
            vyvesti_spravku();
            return 1;
        }
    }
    
    /* Выполнение тестов если запрошено */
    if (zapusk_testov) {
        testirovat_funkcii();
        return 0;
    }
    
    /* Поиск точек пересечения */
    TochkaPeresecheniya tochki[6]; /* Максимум 3 точки для 3 функций */
    int kolvo_tochek = najti_peresecheniya(tochki, 6);
    
    /* Вывод точек пересечения если запрошено */
    if (pechatat_peresecheniya) {
        printf("Точки пересечения:\n");
        for (int i = 0; i < kolvo_tochek; i++) {
            printf("x = %.6f", tochki[i].x);
            if (pechatat_iteracii) {
                printf(" (итераций: %d)", tochki[i].iteracii);
            }
            printf("\n");
        }
        return 0;
    }
    
    /* Вывод количества итераций если запрошено */
    if (pechatat_iteracii) {
        printf("Количество итераций для нахождения точек пересечения:\n");
        for (int i = 0; i < kolvo_tochek; i++) {
            printf("Точка %d: %d итераций\n", i + 1, tochki[i].iteracii);
        }
        return 0;
    }
    
    /* Вычисление и вывод площади */
    double ploshad = vychislit_ploshad();
    printf("Площадь, образуемая функциями: %.6f\n", ploshad);
    
    return 0;
}

/**
 * @brief Находит точки пересечения между функциями
 */
int najti_peresecheniya(TochkaPeresecheniya tochki[], int max_tochek) {
    int schetchik = 0;
    
    /* Точка пересечения lin_func = cub_func */
    if (schetchik < max_tochek) {
        RootResult result = find_root(lin_func, lin_deriv, cub_func, cub_deriv, -10.0, 10.0, TOCHNOST1);
        tochki[schetchik].x = result.koren;
        tochki[schetchik].iteracii = result.iteracii;
        schetchik++;
    }
    
    /* Точка пересечения lin_func = inv_func (корни около 0.854 и -5.854) */
    if (schetchik < max_tochek) {
        /* Используем интервал, где функция меняет знак */
        RootResult result = find_root(lin_func, lin_deriv, inv_func, inv_deriv, 0.5, 2.0, TOCHNOST1);
        if (!result.shlo) {
            /* Пробуем другой интервал для отрицательного корня */
            result = find_root(lin_func, lin_deriv, inv_func, inv_deriv, -7.0, -4.0, TOCHNOST1);
        }
        tochki[schetchik].x = result.koren;
        tochki[schetchik].iteracii = result.iteracii;
        schetchik++;
    }
    
    /* Точка пересечения cub_func = inv_func (корень около 3.5) */
    if (schetchik < max_tochek) {
        RootResult result = find_root(cub_func, cub_deriv, inv_func, inv_deriv, 3.0, 4.0, TOCHNOST1);
        tochki[schetchik].x = result.koren;
        tochki[schetchik].iteracii = result.iteracii;
        schetchik++;
    }
    
    return schetchik;
}

/**
 * @brief Вычисляет площадь, образуемую функциями
 */
double vychislit_ploshad() {
    TochkaPeresecheniya tochki[6];
    int kolvo_tochek = najti_peresecheniya(tochki, 6);
    
    /* Сортировка точек по возрастанию */
    for (int i = 0; i < kolvo_tochek - 1; i++) {
        for (int j = i + 1; j < kolvo_tochek; j++) {
            if (tochki[i].x > tochki[j].x) {
                TochkaPeresecheniya temp = tochki[i];
                tochki[i] = tochki[j];
                tochki[j] = temp;
            }
        }
    }
    
    double ploshad = 0.0;
    
    /* Вычисление площади на интервалах между точками пересечения */
    for (int i = 0; i < kolvo_tochek - 1; i++) {
        double levaya = tochki[i].x;
        double pravaya = tochki[i + 1].x;
        
        /* Определение верхней и нижней функций на интервале */
        double seredina = (levaya + pravaya) / 2.0;
        double lin_seredina = lin_func(seredina);
        double cub_seredina = cub_func(seredina);
        double inv_seredina = inv_func(seredina);
        
        /* Находим верхнюю функцию (максимум из трех) */
        double max_znachenie = lin_seredina;
        double (*verkh_funkc)(double) = lin_func;
        
        if (cub_seredina > max_znachenie) {
            max_znachenie = cub_seredina;
            verkh_funkc = cub_func;
        }
        if (inv_seredina > max_znachenie) {
            max_znachenie = inv_seredina;
            verkh_funkc = inv_func;
        }
        
        /* Находим нижнюю функцию (минимум из трех) */
        double min_znachenie = lin_seredina;
        double (*nizh_funkc)(double) = lin_func;
        
        if (cub_seredina < min_znachenie) {
            min_znachenie = cub_seredina;
            nizh_funkc = cub_func;
        }
        if (inv_seredina < min_znachenie) {
            min_znachenie = inv_seredina;
            nizh_funkc = inv_func;
        }
        
        /* Вычисляем площадь как интеграл от разности */
        double ploshad_segmenta = calc_integral(verkh_funkc, levaya, pravaya, TOCHNOST2) - calc_integral(nizh_funkc, levaya, pravaya, TOCHNOST2);
        ploshad += ploshad_segmenta;
    }
    
    return ploshad;
}

/**
 * @brief Тестирует функции find_root и calc_integral
 */
void testirovat_funkcii() {
    printf("Тестирование функций:\n");
    
    /* Тест функции find_root */
    printf("\nТест функции find_root:\n");
    RootResult result;
    
    /* Тест: lin_func(x) = cub_func(x) */
    result = find_root(lin_func, lin_deriv, cub_func, cub_deriv, -10.0, 10.0, TOCHNOST1);
    printf("lin_func(x) = cub_func(x): x = %.6f, итераций: %d, сошлось: %s\n", 
           result.koren, result.iteracii, result.shlo ? "да" : "нет");
    
    /* Тест: lin_func(x) = inv_func(x) (положительный корень) */
    result = find_root(lin_func, lin_deriv, inv_func, inv_deriv, 0.5, 2.0, TOCHNOST1);
    printf("lin_func(x) = inv_func(x) (положительный корень): x = %.6f, итераций: %d, сошлось: %s\n", 
           result.koren, result.iteracii, result.shlo ? "да" : "нет");
    
    /* Тест: cub_func(x) = inv_func(x) */
    result = find_root(cub_func, cub_deriv, inv_func, inv_deriv, 3.0, 4.0, TOCHNOST1);
    printf("cub_func(x) = inv_func(x): x = %.6f, итераций: %d, сошлось: %s\n", 
           result.koren, result.iteracii, result.shlo ? "да" : "нет");
    
    /* Тест функции calc_integral */
    printf("\nТест функции calc_integral:\n");
    
    /* Тест: интеграл от lin_func на [0, 1] */
    double integral_lin = calc_integral(lin_func, 0.0, 1.0, TOCHNOST2);
    printf("Интеграл lin_func(x) от 0 до 1: %.6f\n", integral_lin);
    
    /* Тест: интеграл от cub_func на [0, 1] */
    double integral_cub = calc_integral(cub_func, 0.0, 1.0, TOCHNOST2);
    printf("Интеграл cub_func(x) от 0 до 1: %.6f\n", integral_cub);
    
    /* Тест: интеграл от inv_func на [1, 2] */
    double integral_inv = calc_integral(inv_func, 1.0, 2.0, TOCHNOST2);
    printf("Интеграл inv_func(x) от 1 до 2: %.6f\n", integral_inv);
}

/**
 * @brief Выводит справку по использованию программы
 */
void vyvesti_spravku() {
    printf("Использование: %s [ключи]\n", "area_calculator");
    printf("Ключи:\n");
    printf("  -help              Вывести эту справку\n");
    printf("  -intersections     Вывести абсциссы точек пересечения кривых\n");
    printf("  -iterations        Вывести количество итераций для нахождения точек\n");
    printf("  -test              Запустить тестирование функций\n");
    printf("  (без ключей)       Вычислить площадь, образуемую функциями\n");
}