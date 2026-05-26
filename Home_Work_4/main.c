#define _XOPEN_SOURCE_EXTENDED 1
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses/ncurses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};
enum {CONTROLS=5}; // Количество типов управления (стрелки + wsad + WSAD + русские строчные + русские заглавные)


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
} food_t;

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
tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
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
    mvprintw(food->y, food->x, "F");
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
 Движение головы с учетом текущего направления движения
 */
int go(struct snake_t *head)
{
    char ch = '@';
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x); // macro - размер терминала
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
    for (int i = 0; i < CONTROLS; i++)
    {
        if (key == default_controls[i].down)
        {
            snake->direction = DOWN;
            break;
        }
        else if (key == default_controls[i].up)
        {
            snake->direction = UP;
            break;
        }
        else if (key == default_controls[i].right)
        {
            snake->direction = RIGHT;
            break;
        }
        else if (key == default_controls[i].left)
        {
            snake->direction = LEFT;
            break;
        }
    }
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
void goTail(struct snake_t *head)
{
    char ch = '*';
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
    setlocale(LC_ALL, ""); // Включаем поддержку локали для кириллицы
    srand(time(NULL));
    
    snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake,START_TAIL_SIZE,10,10);
    
    food_t food = {0, 0, 0, 0};
    
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    //Отключаем курсор
    
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x);
    
    mvprintw(0, 0,"Use arrows for control. Press 'F10' for EXIT");
    
    // Спавним первую еду
    spawnFood(&food, max_x, max_y);
    
    wchar_t key_pressed=0;
    int game_over = 0;
    
    // *** ЗАДАНИЕ 3: Переписать timeout через clock() ***
    clock_t last_update = clock();
    const clock_t frame_time = CLOCKS_PER_SEC / 10; // 10 FPS
    
    nodelay(stdscr, TRUE); // Неблокирующий getch()
    
    while( key_pressed != STOP_GAME && !game_over )
    {
        clock_t now = clock();
        
        // Проверяем, прошло ли достаточно времени для обновления
        if(now - last_update >= frame_time)
        {
            last_update = now;
            
            int result = get_wch(&key_pressed); // Считываем клавишу (wide char)
            if(result == OK || result == KEY_CODE_YES)
            {
                if(result == KEY_CODE_YES)
                {
                    // Специальные клавиши (стрелки, F-клавиши)
                    if(key_pressed == STOP_GAME)
                    {
                        break;
                    }
                }
                // Проверяем корректность направления перед изменением
                if(checkDirection(snake, key_pressed))
                {
                    changeDirection(snake, key_pressed);
                }
            }
            
            // *** ЗАДАНИЕ 2: Выход за границы экрана (смерть) ***
            // Двигаем змею, go() возвращает 0 при столкновении с границей
            if(!go(snake))
            {
                game_over = 1;
                break;
            }
            
            goTail(snake);
            
            // *** ЗАДАНИЕ 1: Змея врезается сама в себя ***
            if(checkSelfCollision(snake))
            {
                game_over = 1;
                break;
            }
            
            // Проверяем съедание еды
            if(checkFoodCollision(snake, &food))
            {
                growTail(snake);
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
            
            // Отображаем счет
            mvprintw(1, 0, "Score: %d", (int)(snake->tsize - START_TAIL_SIZE - 1));
            refresh();
        }
    }
    
    // Сообщение о конце игры
    if(game_over)
    {
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(max_y / 2, max_x / 2 - 10, "GAME OVER! Score: %d", 
                 (int)(snake->tsize - START_TAIL_SIZE - 1));
        mvprintw(max_y / 2 + 1, max_x / 2 - 15, "Press F10 to exit");
        refresh();
        
        // Ждем нажатия F10
        while(key_pressed != STOP_GAME)
        {
            get_wch(&key_pressed);
            napms(100);
        }
    }
    
    free(snake->tail);
    free(snake);
    endwin(); // Завершаем режим curses mod
    return 0;
}
