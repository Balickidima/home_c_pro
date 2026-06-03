#define _XOPEN_SOURCE_EXTENDED 1
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ncurses/ncurses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};
enum {CONTROLS=5}; // Количество типов управления (стрелки + wsad + WSAD + русские строчные + русские заглавные)
enum {SNAKE_COLOR_COUNT=7}; // Количество доступных цветов для змеек
enum {FOOD_TYPE_NORMAL=0, FOOD_TYPE_SPEED=1, FOOD_TYPE_SLOW=2}; // Типы еды


// Здесь храним коды управления змейкой (используем wchar_t для поддержки кириллицы)
struct control_buttons
{
    wchar_t down;
    wchar_t up;
    wchar_t left;
    wchar_t right;
};

// Массив с типами управления: 0 - стрелки, 1 - wsad строчные, 2 - WSAD заглавные,
// 3 - русские строчные (ц ы ф в), 4 - русские заглавные (Ц Ы Ф В)
// Используем явные Unicode коды для надёжности
struct control_buttons default_controls[CONTROLS] = {
    {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},    // Стрелки
    {L's', L'w', L'a', L'd'},                    // wsad (строчные латинские)
    {L'S', L'W', L'A', L'D'},                    // WSAD (заглавные латинские)
    {0x044B, 0x0446, 0x0444, 0x0432},            // ыцфв (строчные русские: ы=down, ц=up, ф=left, в=right)
    {0x042B, 0x0426, 0x0424, 0x0412}             // ЫЦФВ (заглавные русские: Ы=down, Ц=up, Ф=left, В=right)
};

/*
 Голова змейки содержит в себе
 x,y - координаты текущей позиции
 direction - направление движения
 tsize - размер хвоста
 *tail -  ссылка на хвост
 */
typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    int color_pair; // Цвет змейки (1-7)
} snake_t;

/*
 Хвост это массив состоящий из координат x,y
 */
typedef struct tail_t
{
    int x;
    int y;
} tail_t;

/*
 Еда с координатами и таймером жизни
 */
typedef struct food_t
{
    int x;
    int y;
    time_t spawn_time;
    int active;
    int type; // 0 - обычная, 1 - ускорение, 2 - замедление
} food_t;

/*
 Установка цвета для объектов игры
 objectType: 1 - змейка 1, 2 - змейка 2, 3 - обычная еда, 
             4 - бонусная еда (ускорение), 5 - бонусная еда (замедление)
 */
void setColor(int objectType)
{
    attroff(COLOR_PAIR(1));
    attroff(COLOR_PAIR(2));
    attroff(COLOR_PAIR(3));
    attroff(COLOR_PAIR(4));
    attroff(COLOR_PAIR(5));
    
    switch(objectType)
    {
        case 1: // SNAKE1
            attron(COLOR_PAIR(1));
            break;
        case 2: // SNAKE2
            attron(COLOR_PAIR(2));
            break;
        case 3: // FOOD_NORMAL
            attron(COLOR_PAIR(3));
            break;
        case 4: // FOOD_BONUS_SPEED
            attron(COLOR_PAIR(4));
            break;
        case 5: // FOOD_BONUS_SLOW
            attron(COLOR_PAIR(5));
            break;
    }
}

/*
 Анимация звёздочек на главном экране
 */
void drawStarAnimation(int frame, int max_y, int max_x)
{
    int center_y = max_y / 2;
    int center_x = max_x / 2;
    int radius = 8;
    
    for(int i = 0; i < 8; i++)
    {
        double angle = (frame * 0.1) + (i * 3.14159 / 4);
        int y = center_y + (int)(radius * sin(angle));
        int x = center_x + (int)(radius * 1.8 * cos(angle));
        
        if(y > 0 && y < max_y - 1 && x > 0 && x < max_x - 1)
        {
            attron(COLOR_PAIR(4) | A_BOLD);
            mvprintw(y, x, "*");
            attroff(COLOR_PAIR(4) | A_BOLD);
        }
    }
    
    // Внутренний круг
    for(int i = 0; i < 5; i++)
    {
        double angle = -(frame * 0.15) + (i * 3.14159 * 2 / 5);
        int y = center_y + (int)(radius * 0.5 * sin(angle));
        int x = center_x + (int)(radius * 0.9 * cos(angle));
        
        if(y > 0 && y < max_y - 1 && x > 0 && x < max_x - 1)
        {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(y, x, "*");
            attroff(COLOR_PAIR(3) | A_BOLD);
        }
    }
}

/*
 Главный экран приветствия (статичный, без анимации)
 Возвращает 0 если ENTER, 1 если F10
 */
int welcomeScreen(void)
{
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    raw();
    noecho();
    nodelay(stdscr, FALSE);

    wchar_t key = 0;

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    clear();

    attron(A_BOLD | COLOR_PAIR(4));
    mvwaddnwstr(stdscr, max_y/2 - 5, max_x/2 - 18, L"====================================", -1);
    mvwaddnwstr(stdscr, max_y/2 - 4, max_x/2 - 10, L"SNAKE GAME", -1);
    mvwaddnwstr(stdscr, max_y/2 - 3, max_x/2 - 18, L"====================================", -1);
    attroff(A_BOLD | COLOR_PAIR(4));

    attron(A_DIM);
    mvwaddnwstr(stdscr, max_y/2 - 1, max_x/2 - 8, L"v2.0 - 2 Players", -1);
    attroff(A_DIM);

    attron(A_BOLD | COLOR_PAIR(3));
    mvwaddnwstr(stdscr, max_y/2 + 2, max_x/2 - 10, L"[ENTER] - ИГРАТЬ", -1);
    attroff(A_BOLD | COLOR_PAIR(3));

    attron(A_BOLD);
    mvwaddnwstr(stdscr, max_y/2 + 4, max_x/2 - 8, L"[F10] - ВЫХОД", -1);
    attroff(A_BOLD);

    refresh();

    while(key != '\n' && key != KEY_ENTER && key != STOP_GAME)
    {
        get_wch(&key);
        if(key == STOP_GAME)
        {
            return 1;
        }
    }

    return 0;
}

/*
 Пошаговое меню настроек игры
 Возвращает 1 если ESC (вернуться в главное меню), 0 если успешно
 */
int startMenu(int *numPlayers, int *gameMode, int *snakeColor1, int *snakeColor2, int *hardcoreMode)
{
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    raw();
    noecho();
    nodelay(stdscr, FALSE);

    int color_options[SNAKE_COLOR_COUNT] = {
        COLOR_RED, COLOR_BLUE, COLOR_GREEN,
        COLOR_YELLOW, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
    };
    wchar_t* color_names[SNAKE_COLOR_COUNT] = {
        L"Красный", L"Синий", L"Зелёный",
        L"Жёлтый", L"Маджента", L"Голубой", L"Белый"
    };

    wchar_t key = 0;
    int selection = 0;
    int step = 1;

    while(step <= 5)
    {
        clear();

        if(step == 1)
        {
            attron(A_BOLD | COLOR_PAIR(4));
            mvwaddnwstr(stdscr, 1, 5, L"===========================================", -1);
            mvwaddnwstr(stdscr, 2, 10, L"ШАГ 1/4: КОЛИЧЕСТВО ИГРОКОВ", -1);
            mvwaddnwstr(stdscr, 3, 5, L"===========================================", -1);
            attroff(A_BOLD | COLOR_PAIR(4));

            if(selection == 0) attron(A_REVERSE);
            mvwaddnwstr(stdscr, 6, 8, L"1 - Один игрок (WASD)", -1);
            if(selection == 0) attroff(A_REVERSE);

            if(selection == 1) attron(A_REVERSE);
            mvwaddnwstr(stdscr, 8, 8, L"2 - Два игрока (WASD + Стрелки)", -1);
            if(selection == 1) attroff(A_REVERSE);

            attron(A_BOLD);
            mvwaddnwstr(stdscr, 12, 5, L"UP/DOWN - выбор, ENTER - подтвердить", -1);
            mvwaddnwstr(stdscr, 13, 5, L"ESC - вернуться в главное меню", -1);
            attroff(A_BOLD);

            refresh();
            get_wch(&key);

            if(key == 27) return 1;
            if(key == KEY_UP && selection > 0) selection--;
            if(key == KEY_DOWN && selection < 1) selection++;
            if(key == '\n' || key == KEY_ENTER)
            {
                *numPlayers = selection + 1;
                selection = 0;
                step = 2;
            }
        }
        else if(step == 2)
        {
            attron(A_BOLD | COLOR_PAIR(4));
            mvwaddnwstr(stdscr, 1, 5, L"===========================================", -1);
            mvwaddnwstr(stdscr, 2, 10, L"ШАГ 2/4: РЕЖИМ ИГРЫ", -1);
            mvwaddnwstr(stdscr, 3, 5, L"===========================================", -1);
            attroff(A_BOLD | COLOR_PAIR(4));

            if(selection == 0) attron(A_REVERSE);
            mvwaddnwstr(stdscr, 6, 8, L"1 - Обычный (без смерти от столкновений)", -1);
            if(selection == 0) attroff(A_REVERSE);

            if(selection == 1) attron(A_REVERSE);
            mvwaddnwstr(stdscr, 8, 8, L"2 - Хардкор (столкновения змеек запрещены)", -1);
            if(selection == 1) attroff(A_REVERSE);

            attron(A_BOLD);
            mvwaddnwstr(stdscr, 12, 5, L"UP/DOWN - выбор, ENTER - подтвердить", -1);
            mvwaddnwstr(stdscr, 13, 5, L"ESC - вернуться в главное меню", -1);
            attroff(A_BOLD);

            refresh();
            get_wch(&key);

            if(key == 27) return 1;
            if(key == KEY_UP && selection > 0) selection--;
            if(key == KEY_DOWN && selection < 1) selection++;
            if(key == '\n' || key == KEY_ENTER)
            {
                *hardcoreMode = selection; // 0 - обычный, 1 - хардкор
                selection = 0;
                step = 3;
            }
        }
        else if(step == 3)
        {
            attron(A_BOLD | COLOR_PAIR(4));
            mvwaddnwstr(stdscr, 1, 5, L"===========================================", -1);
            mvwaddnwstr(stdscr, 2, 10, L"ШАГ 3/4: ЦВЕТ ЗМЕЙКИ 1", -1);
            mvwaddnwstr(stdscr, 3, 5, L"===========================================", -1);
            attroff(A_BOLD | COLOR_PAIR(4));

            for(int i = 0; i < SNAKE_COLOR_COUNT; i++)
            {
                if(selection == i) attron(A_REVERSE);
                init_pair(10 + i, color_options[i], COLOR_BLACK);
                attron(COLOR_PAIR(10 + i));
                mvprintw(6 + i, 10, "%d - ", i+1);
                mvwaddnwstr(stdscr, 6 + i, 14, color_names[i], -1);
                attroff(COLOR_PAIR(10 + i));
                if(selection == i) attroff(A_REVERSE);
            }

            attron(A_BOLD);
            mvwaddnwstr(stdscr, 15, 5, L"UP/DOWN - выбор, ENTER - подтвердить", -1);
            mvwaddnwstr(stdscr, 16, 5, L"ESC - вернуться в главное меню", -1);
            attroff(A_BOLD);

            refresh();
            get_wch(&key);

            if(key == 27) return 1;
            if(key == KEY_UP && selection > 0) selection--;
            if(key == KEY_DOWN && selection < SNAKE_COLOR_COUNT - 1) selection++;
            if(key == '\n' || key == KEY_ENTER)
            {
                *snakeColor1 = selection + 1;
                selection = 0;
                step = 4;
            }
        }
        else if(step == 4)
        {
            if(*numPlayers == 2)
            {
                attron(A_BOLD | COLOR_PAIR(4));
                mvwaddnwstr(stdscr, 1, 5, L"===========================================", -1);
                mvwaddnwstr(stdscr, 2, 10, L"ШАГ 4a/4: ЦВЕТ ЗМЕЙКИ 2", -1);
                mvwaddnwstr(stdscr, 3, 5, L"===========================================", -1);
                attroff(A_BOLD | COLOR_PAIR(4));

                for(int i = 0; i < SNAKE_COLOR_COUNT; i++)
                {
                    if(selection == i) attron(A_REVERSE);
                    init_pair(20 + i, color_options[i], COLOR_BLACK);
                    attron(COLOR_PAIR(20 + i));
                    mvprintw(6 + i, 10, "%d - ", i+1);
                    mvwaddnwstr(stdscr, 6 + i, 14, color_names[i], -1);
                    attroff(COLOR_PAIR(20 + i));
                    if(selection == i) attroff(A_REVERSE);
                }

                attron(A_BOLD);
                mvwaddnwstr(stdscr, 15, 5, L"UP/DOWN - выбор, ENTER - подтвердить", -1);
                mvwaddnwstr(stdscr, 16, 5, L"ESC - вернуться в главное меню", -1);
                attroff(A_BOLD);

                refresh();
                get_wch(&key);

                if(key == 27) return 1;
                if(key == KEY_UP && selection > 0) selection--;
                if(key == KEY_DOWN && selection < SNAKE_COLOR_COUNT - 1) selection++;
                if(key == '\n' || key == KEY_ENTER)
                {
                    *snakeColor2 = selection + 1;
                    selection = 0;
                    step = 5;
                }
            }
            else
            {
                *snakeColor2 = 2;
                step = 5;
            }
        }
        else if(step == 5)
        {
            attron(A_BOLD | COLOR_PAIR(4));
            mvwaddnwstr(stdscr, 1, 5, L"===========================================", -1);
            mvwaddnwstr(stdscr, 2, 10, L"ШАГ 4/4: СЛОЖНОСТЬ", -1);
            mvwaddnwstr(stdscr, 3, 5, L"===========================================", -1);
            attroff(A_BOLD | COLOR_PAIR(4));

            if(selection == 0) attron(A_REVERSE);
            mvwaddnwstr(stdscr, 6, 8, L"1 - Лёгкая (медленная скорость)", -1);
            if(selection == 0) attroff(A_REVERSE);

            if(selection == 1) attron(A_REVERSE);
            mvwaddnwstr(stdscr, 8, 8, L"2 - Средняя (обычная скорость)", -1);
            if(selection == 1) attroff(A_REVERSE);

            if(selection == 2) attron(A_REVERSE);
            mvwaddnwstr(stdscr, 10, 8, L"3 - Сложная (быстрая скорость)", -1);
            if(selection == 2) attroff(A_REVERSE);

            attron(A_BOLD);
            mvwaddnwstr(stdscr, 13, 5, L"UP/DOWN - выбор, ENTER - начать игру", -1);
            mvwaddnwstr(stdscr, 14, 5, L"ESC - вернуться в главное меню", -1);
            attroff(A_BOLD);

            refresh();
            get_wch(&key);

            if(key == 27) return 1;
            if(key == KEY_UP && selection > 0) selection--;
            if(key == KEY_DOWN && selection < 2) selection++;
            if(key == '\n' || key == KEY_ENTER)
            {
                *gameMode = selection + 1;
                return 0;
            }
        }
    }

    return 0;
}

/*
 Экран паузы с подтверждением выхода
 Возвращает 1 если выйти в главное меню, 0 если продолжить
 */
int pauseMenu(void)
{
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    raw();
    noecho();
    nodelay(stdscr, FALSE);

    int selection = 0;
    wchar_t key = 0;

    while(key != '\n' && key != KEY_ENTER)
    {
        clear();

        attron(A_BOLD | COLOR_PAIR(4));
        mvwaddnwstr(stdscr, 10, 5, L"===========================================", -1);
        mvwaddnwstr(stdscr, 11, 16, L"ПАУЗА", -1);
        mvwaddnwstr(stdscr, 12, 5, L"===========================================", -1);
        attroff(A_BOLD | COLOR_PAIR(4));

        if(selection == 0) attron(A_REVERSE);
        mvwaddnwstr(stdscr, 14, 10, L"Продолжить игру", -1);
        if(selection == 0) attroff(A_REVERSE);

        if(selection == 1) attron(A_REVERSE);
        mvwaddnwstr(stdscr, 16, 10, L"Вернуться в главное меню", -1);
        if(selection == 1) attroff(A_REVERSE);

        attron(A_BOLD);
        mvwaddnwstr(stdscr, 19, 5, L"UP/DOWN - выбор, ENTER - подтвердить", -1);
        attroff(A_BOLD);

        refresh();
        get_wch(&key);

        if(key == KEY_UP && selection > 0) selection--;
        if(key == KEY_DOWN && selection < 1) selection++;
    }

    return selection == 1 ? 1 : 0;
}

void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}
void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
}

void initSnake(snake_t *head, size_t size, int x, int y)
{
    tail_t* tail = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    if (tail == NULL) {
        endwin();
        fprintf(stderr, "Memory allocation failed for snake tail\n");
        exit(1);
    }
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size+1;
}

/*
 Генерация еды на случайной позиции
 */
void spawnFood(food_t *food, int max_x, int max_y)
{
    food->x = rand() % (max_x - 2) + 1;
    food->y = rand() % (max_y - MIN_Y - 1) + MIN_Y + 1;
    food->spawn_time = time(NULL);
    food->active = 1;
    
    // 60% обычная, 20% ускорение, 20% замедление
    int bonus_type = rand() % 100;
    if(bonus_type < 60) {
        food->type = FOOD_TYPE_NORMAL;
    } else if(bonus_type < 80) {
        food->type = FOOD_TYPE_SPEED;
    } else {
        food->type = FOOD_TYPE_SLOW;
    }
    
    // Отрисовка в зависимости от типа
    switch(food->type) {
        case FOOD_TYPE_NORMAL:
            setColor(3);
            mvprintw(food->y, food->x, "F");
            break;
        case FOOD_TYPE_SPEED:
            setColor(4);
            mvprintw(food->y, food->x, "S");
            break;
        case FOOD_TYPE_SLOW:
            setColor(5);
            mvprintw(food->y, food->x, "L");
            break;
    }
}

/*
 Проверка столкновения змеи с самой собой
 */
int checkSelfCollision(struct snake_t *head)
{
    // Проверяем, совпадает ли голова с любой частью хвоста
    for(size_t i = 1; i < head->tsize - 1; i++)
    {
        if(head->tail[i].x == head->x && head->tail[i].y == head->y)
        {
            return 1;
        }
    }
    return 0;
}

/*
 Проверка выхода за границы экрана
 */
int checkBorderCollision(struct snake_t *head, int max_x, int max_y)
{
    if(head->x <= 0 || head->x >= max_x - 1 || 
       head->y <= 0 || head->y >= max_y - 1)
    {
        return 1;
    }
    return 0;
}

/*
 Проверка съедания еды
 */
int checkFoodCollision(struct snake_t *head, food_t *food)
{
    if(food->active && head->x == food->x && head->y == food->y)
    {
        return 1;
    }
    return 0;
}

/*
 Проверка столкновения двух змеек
 */
int checkSnakeCollision(struct snake_t *snake1, struct snake_t *snake2)
{
    // Проверяем, не сталкивается ли голова первой змейки с телом второй
    for(size_t i = 0; i < snake2->tsize; i++)
    {
        if(snake1->x == snake2->tail[i].x && snake1->y == snake2->tail[i].y)
        {
            return 1;
        }
    }
    
    // Проверяем, не сталкивается ли голова второй змейки с телом первой
    for(size_t i = 0; i < snake1->tsize; i++)
    {
        if(snake2->x == snake1->tail[i].x && snake2->y == snake1->tail[i].y)
        {
            return 1;
        }
    }
    
    return 0;
}

/*
 Движение головы с учетом текущего направления движения
 */
int go(struct snake_t *head, int color_type)
{
    char ch = '@';
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x); // macro - размер терминала
    
    // Устанавливаем цвет
    setColor(color_type);
    mvprintw(head->y, head->x, " "); // очищаем один символ
    switch (head->direction)
    {
        case LEFT:
            mvprintw(head->y, --(head->x), "%c", ch);
        break;
        case RIGHT:
            mvprintw(head->y, ++(head->x), "%c", ch);
        break;
        case UP:
            mvprintw(--(head->y), head->x, "%c", ch);
        break;
        case DOWN:
            mvprintw(++(head->y), head->x, "%c", ch);
        break;
        default:
        break;
    }
    refresh();

    // Проверяем столкновение с границей
    if(checkBorderCollision(head, max_x, max_y))
    {
        return 0;
    }
    return 1;
}

void changeDirection(struct snake_t* snake, const wchar_t key)
{
    int new_direction = 0;
    
    for (int i = 0; i < CONTROLS; i++)
    {
        if (key == default_controls[i].down)
        {
            new_direction = DOWN;
            break;
        }
        else if (key == default_controls[i].up)
        {
            new_direction = UP;
            break;
        }
        else if (key == default_controls[i].right)
        {
            new_direction = RIGHT;
            break;
        }
        else if (key == default_controls[i].left)
        {
            new_direction = LEFT;
            break;
        }
    }
    
    // Если клавиша не соответствует никакому направлению, выходим
    if (new_direction == 0)
        return;
    
    // Запрещаем разворот на 180 градусов
    if (snake->direction == RIGHT && new_direction == LEFT) return;
    if (snake->direction == UP && new_direction == DOWN) return;
    if (snake->direction == LEFT && new_direction == RIGHT) return;
    if (snake->direction == DOWN && new_direction == UP) return;
    
    snake->direction = new_direction;
}

/*
 Проверка корректности выбранного направления.
 Змейка не может разворачиваться на 180 градусов.
 Возвращает 1 если направление корректно, 0 если недопустимо.
 */
int checkDirection(snake_t* snake, wchar_t key)
{
    // Определяем, какое направление соответствует нажатой клавише
    int new_direction = 0;

    for (int i = 0; i < CONTROLS; i++)
    {
        if (key == default_controls[i].down)
        {
            new_direction = DOWN;
            break;
        }
        else if (key == default_controls[i].up)
        {
            new_direction = UP;
            break;
        }
        else if (key == default_controls[i].right)
        {
            new_direction = RIGHT;
            break;
        }
        else if (key == default_controls[i].left)
        {
            new_direction = LEFT;
            break;
        }
    }

    // Если клавиша не соответствует никакому направлению, разрешаем
    if (new_direction == 0)
        return 1;

    // Запрещаем разворот на 180 градусов
    if (snake->direction == RIGHT && new_direction == LEFT)
        return 0;
    if (snake->direction == UP && new_direction == DOWN)
        return 0;
    if (snake->direction == LEFT && new_direction == RIGHT)
        return 0;
    if (snake->direction == DOWN && new_direction == UP)
        return 0;

    return 1;
}

/*
 Движение хвоста с учетом движения головы
 */
void goTail(struct snake_t *head, int color_type)
{
    char ch = '*';
    
    // Устанавливаем цвет
    setColor(color_type);
    
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

/*
 Увеличение хвоста на 1
 */
void growTail(struct snake_t *head)
{
    if(head->tsize < MAX_TAIL_SIZE)
    {
        head->tail[head->tsize - 1].x = head->tail[head->tsize - 2].x;
        head->tail[head->tsize - 1].y = head->tail[head->tsize - 2].y;
        head->tsize++;
    }
}

int main()
{
    // Переключаем консоль в UTF-8 (для Windows)
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
    
    // Включаем поддержку UTF-8 локали
    setlocale(LC_ALL, "");
    srand(time(NULL));

    // Переменные для настроек игры
    int numPlayers = 2;      // 1 или 2 игрока
    int gameMode = 2;        // 1 - лёгкая, 2 - средняя, 3 - сложная
    int snakeColor1 = 1;     // Красный
    int snakeColor2 = 2;     // Синий
    
    // Цвета для инициализации
    int color_options[SNAKE_COLOR_COUNT] = {
        COLOR_RED, COLOR_BLUE, COLOR_GREEN, 
        COLOR_YELLOW, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
    };

    initscr();
    
    // Инициализация цветов
    start_color();
    init_pair(1, color_options[0], COLOR_BLACK); // Змейка 1 (по умолчанию красный)
    init_pair(2, color_options[1], COLOR_BLACK); // Змейка 2 (по умолчанию синий)
    init_pair(3, COLOR_GREEN, COLOR_BLACK);      // Еда обычная
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);     // Бонусная еда / анимация
    init_pair(5, COLOR_CYAN, COLOR_BLACK);       // Бонусная еда (замедление)

    // Показываем экран приветствия с анимацией
    if(welcomeScreen() == 1)
    {
        // Пользователь нажал F10 - выходим
        endwin();
        return 0;
    }
    
    // Переменная для хардкор режима
    int hardcoreMode = 0; // 0 - обычный, 1 - хардкор

    // Вызываем пошаговое меню настроек
    if(startMenu(&numPlayers, &gameMode, &snakeColor1, &snakeColor2, &hardcoreMode) == 1)
    {
        // ESC - вернуться в главное меню
        if(welcomeScreen() == 1) {
            endwin();
            return 0;
        }
        // Повторно вызываем меню
        startMenu(&numPlayers, &gameMode, &snakeColor1, &snakeColor2, &hardcoreMode);
    }

    // Переинициализируем цвета змеек на основе выбора пользователя
    init_pair(1, color_options[snakeColor1 - 1], COLOR_BLACK);
    init_pair(2, color_options[snakeColor2 - 1], COLOR_BLACK);
    
    // Очищаем экран от текста меню
    clear();
    refresh();

    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Отключаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    // Отключаем курсор

    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x);

    // Создаем змейки
    snake_t* snake1 = (snake_t*)malloc(sizeof(snake_t));
    if (snake1 == NULL) {
        endwin();
        fprintf(stderr, "Memory allocation failed for snake1\n");
        exit(1);
    }
    initSnake(snake1, START_TAIL_SIZE, 10, 10);
    snake1->color_pair = 1;

    snake_t* snake2 = NULL;
    if(numPlayers == 2)
    {
        snake2 = (snake_t*)malloc(sizeof(snake_t));
        if (snake2 == NULL) {
            endwin();
            fprintf(stderr, "Memory allocation failed for snake2\n");
            free(snake1->tail);
            free(snake1);
            exit(1);
        }
        initSnake(snake2, START_TAIL_SIZE, max_x/2 + 5, max_y/2 + 5);
        snake2->color_pair = 2;
    }
    
    food_t food = {0, 0, 0, 0, FOOD_TYPE_NORMAL};

    mvprintw(0, 0, "P1: WASD/ЦФЫВ");
    if(numPlayers == 2)
    {
        mvprintw(0, 20, "| P2: Стрелки");
    }
    mvprintw(0, 40, "| F10 - EXIT");

    // Спавним первую еду
    spawnFood(&food, max_x, max_y);

    wchar_t key_pressed=0;
    int game_over = 0;
    int level = 1;
    clock_t bonus1_speed_timer = 0; // Таймер для бонуса скорости змейки 1
    clock_t bonus2_speed_timer = 0; // Таймер для бонуса скорости змейки 2

    // Базовая скорость зависит от сложности (повышено до 24 FPS)
    clock_t base_frame_time;
    switch(gameMode) {
        case 1: base_frame_time = CLOCKS_PER_SEC / 12; // Лёгкая - 12 FPS
            break;
        case 2: base_frame_time = CLOCKS_PER_SEC / 18; // Средняя - 18 FPS
            break;
        case 3: base_frame_time = CLOCKS_PER_SEC / 24; // Сложная - 24 FPS
            break;
        default: base_frame_time = CLOCKS_PER_SEC / 18;
    }

    clock_t current_frame_time = base_frame_time;
    clock_t last_update = clock();

    nodelay(stdscr, TRUE); // Неблокирующий getch()

    while( key_pressed != STOP_GAME && !game_over )
    {
        clock_t now = clock();

        // Обрабатываем ВСЕ нажатия клавиш (цикл для очистки буфера)
        int result;
        while((result = get_wch(&key_pressed)) != ERR)
        {
            if(result == KEY_CODE_YES)
            {
                // Специальные клавиши (стрелки, F-клавиши)
                if(key_pressed == STOP_GAME)
                {
                    goto game_end;
                }
            }

            // ESC - пауза
            if(key_pressed == 27)
            {
                if(pauseMenu() == 1)
                {
                    // Вернуться в главное меню
                    endwin();
                    // Освобождаем память
                    free(snake1->tail);
                    free(snake1);
                    if(snake2 != NULL) {
                        free(snake2->tail);
                        free(snake2);
                    }
                    // Рекурсивно перезапускаем main
                    return main();
                }
                // После паузы - очищаем экран
                clear();
                refresh();
            }
            else
            {
                // Игрок 1 - WASD и русские клавиши (НЕ стрелки)
                if(key_pressed != KEY_UP && key_pressed != KEY_DOWN &&
                   key_pressed != KEY_LEFT && key_pressed != KEY_RIGHT)
                {
                    changeDirection(snake1, key_pressed);
                }

                // Игрок 2 - только стрелки (если 2 игрока)
                if(numPlayers == 2)
                {
                    if(key_pressed == KEY_UP || key_pressed == KEY_DOWN ||
                       key_pressed == KEY_LEFT || key_pressed == KEY_RIGHT)
                    {
                        changeDirection(snake2, key_pressed);
                    }
                }
            }
        }

        // Проверяем, прошло ли достаточно времени для обновления
        if(now - last_update >= current_frame_time)
        {
            clock_t frame_time = now - last_update;
            last_update = now;

            // Обновляем таймеры бонусов и сбрасываем скорость при их истечении
            if(bonus1_speed_timer > 0)
            {
                if(bonus1_speed_timer <= frame_time)
                {
                    // Таймер истек, возвращаем нормальную скорость
                    bonus1_speed_timer = 0;
                    current_frame_time = base_frame_time - (level - 1) * (CLOCKS_PER_SEC / 50);
                    if(current_frame_time < CLOCKS_PER_SEC / 20) {
                        current_frame_time = CLOCKS_PER_SEC / 20;
                    }
                }
                else
                {
                    // Уменьшаем таймер
                    bonus1_speed_timer -= frame_time;
                }
            }
            
            if(bonus2_speed_timer > 0)
            {
                if(bonus2_speed_timer <= frame_time)
                {
                    // Таймер истек, возвращаем нормальную скорость
                    bonus2_speed_timer = 0;
                    current_frame_time = base_frame_time - (level - 1) * (CLOCKS_PER_SEC / 50);
                    if(current_frame_time < CLOCKS_PER_SEC / 20) {
                        current_frame_time = CLOCKS_PER_SEC / 20;
                    }
                }
                else
                {
                    // Уменьшаем таймер
                    bonus2_speed_timer -= frame_time;
                }
            }

            // Двигаем змейки
            if(!go(snake1, 1)) // Цвет змейки 1
            {
                game_over = 1;
                break;
            }

            if(numPlayers == 2)
            {
                if(!go(snake2, 2)) // Цвет змейки 2
                {
                    game_over = 1;
                    break;
                }
            }

            goTail(snake1, 1);

            if(numPlayers == 2)
            {
                goTail(snake2, 2);
            }

            // Проверяем столкновения в зависимости от режима игры
            if (hardcoreMode) {
                // Хардкор режим: проверяем столкновения змеек
                if (checkSnakeCollision(snake1, snake2)) {
                    game_over = 1;
                    break;
                }
            }
            // *** ЗАДАНИЕ 1: Отключена смерть от самопересечения ***
            // Змейки проходят сквозь себя (но не сквозь друг друга в хардкор режиме)

            // Проверяем съедание еды для змейки 1
            if(checkFoodCollision(snake1, &food))
            {
                growTail(snake1);

                // Применяем эффект бонуса
                if(food.type == FOOD_TYPE_SPEED) {
                    // Ускорение: увеличиваем скорость на 50% на 5 секунд
                    bonus1_speed_timer = 5 * CLOCKS_PER_SEC; // 5 секунд
                    current_frame_time = base_frame_time / 2; // Удвоение скорости
                } else if(food.type == FOOD_TYPE_SLOW) {
                    // Замедление: уменьшаем скорость на 50% на 5 секунд
                    bonus1_speed_timer = 5 * CLOCKS_PER_SEC; // 5 секунд
                    current_frame_time = base_frame_time * 2; // Уменьшение скорости вдвое
                } else {
                    // Обычная еда: сбрасываем к базовой скорости с учетом уровня
                    current_frame_time = base_frame_time - (level - 1) * (CLOCKS_PER_SEC / 50);
                    if(current_frame_time < CLOCKS_PER_SEC / 20) {
                        current_frame_time = CLOCKS_PER_SEC / 20;
                    }
                }

                // Спавним новую еду
                spawnFood(&food, max_x, max_y);
            }

            // Проверяем съедание еды для змейки 2
            if(numPlayers == 2 && checkFoodCollision(snake2, &food))
            {
                growTail(snake2);

                // Применяем эффект бонуса
                if(food.type == FOOD_TYPE_SPEED) {
                    // Ускорение: увеличиваем скорость на 50% на 5 секунд
                    bonus2_speed_timer = 5 * CLOCKS_PER_SEC; // 5 секунд
                    current_frame_time = base_frame_time / 2; // Удвоение скорости
                } else if(food.type == FOOD_TYPE_SLOW) {
                    // Замедление: уменьшаем скорость на 50% на 5 секунд
                    bonus2_speed_timer = 5 * CLOCKS_PER_SEC; // 5 секунд
                    current_frame_time = base_frame_time * 2; // Уменьшение скорости вдвое
                } else {
                    // Обычная еда: сбрасываем к базовой скорости с учетом уровня
                    current_frame_time = base_frame_time - (level - 1) * (CLOCKS_PER_SEC / 50);
                    if(current_frame_time < CLOCKS_PER_SEC / 20) {
                        current_frame_time = CLOCKS_PER_SEC / 20;
                    }
                }

                // Спавним новую еду
                spawnFood(&food, max_x, max_y);
            }

            // Проверяем истечение времени еды
            if(food.active && difftime(time(NULL), food.spawn_time) > FOOD_EXPIRE_SECONDS)
            {
                mvprintw(food.y, food.x, " ");
                food.active = 0;
                // Спавним новую еду
                spawnFood(&food, max_x, max_y);
            }

            // Система уровней
            int total_score = (snake1->tsize - START_TAIL_SIZE - 1);
            if(numPlayers == 2) {
                total_score += (snake2->tsize - START_TAIL_SIZE - 1);
            }

            if(total_score >= level * 10) {
                level++;
                // Увеличиваем скорость
                current_frame_time = base_frame_time - (level - 1) * (CLOCKS_PER_SEC / 50);

                // Минимальный frame_time = CLOCKS_PER_SEC / 20 (20 FPS)
                if(current_frame_time < CLOCKS_PER_SEC / 20) {
                    current_frame_time = CLOCKS_PER_SEC / 20;
                }

                // Отображаем уведомление
                mvprintw(3, 0, "LEVEL UP! Level: %d", level);
            }

            // Отображаем статистику
            if(numPlayers == 2) {
                mvprintw(1, 0, "Level: %d | P1: %d | P2: %d",
                         level,
                         (int)(snake1->tsize - START_TAIL_SIZE - 1),
                         (int)(snake2->tsize - START_TAIL_SIZE - 1));
            } else {
                mvprintw(1, 0, "Level: %d | Score: %d",
                         level,
                         (int)(snake1->tsize - START_TAIL_SIZE - 1));
            }
            refresh();
        }
    }
game_end:

    // Сообщение о конце игры
    if(game_over)
    {
        getmaxyx(stdscr, max_y, max_x);
        if(numPlayers == 2) {
            mvprintw(max_y / 2, max_x / 2 - 12, "GAME OVER! P1: %d | P2: %d",
                     (int)(snake1->tsize - START_TAIL_SIZE - 1),
                     (int)(snake2->tsize - START_TAIL_SIZE - 1));
        } else {
            mvprintw(max_y / 2, max_x / 2 - 10, "GAME OVER! Score: %d",
                     (int)(snake1->tsize - START_TAIL_SIZE - 1));
        }
        mvwaddnwstr(stdscr, max_y / 2 + 1, max_x / 2 - 22, L"Нажмите ENTER для возврата в главное меню", -1);
        refresh();

        // Ждем нажатия ENTER или F10
        nodelay(stdscr, FALSE);
        while(key_pressed != '\n' && key_pressed != KEY_ENTER && key_pressed != STOP_GAME)
        {
            get_wch(&key_pressed);
            napms(100);
        }

        if(key_pressed == '\n' || key_pressed == KEY_ENTER)
        {
            // Вернуться в главное меню
            endwin();
            free(snake1->tail);
            free(snake1);
            if(snake2 != NULL) {
                free(snake2->tail);
                free(snake2);
            }
            return main();
        }
    }

    free(snake1->tail);
    free(snake1);
    
    if(snake2 != NULL)
    {
        free(snake2->tail);
        free(snake2);
    }
    
    endwin(); // Завершаем режим curses mod
    return 0;
}
