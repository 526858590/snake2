#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

// 常量定义
#define MAP_WIDTH 80   // 增加地图宽度
#define MAP_HEIGHT 25  // 增加地图高度
#define INIT_SNAKE_LENGTH 5
#define MAX_SNAKE_LENGTH 100
#define MAX_FOOD 5     // 最大食物数量
#define MIN_FOOD 1     // 最小食物数量
#define FOOD_INTERVAL_MIN 3000  // 最小食物生成间隔(毫秒)
#define FOOD_INTERVAL_MAX 8000  // 最大食物生成间隔(毫秒)

// 方向定义
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// 颜色定义
#define COLOR_SNAKE 14  // 黄色
#define COLOR_FOOD 12   // 红色
#define COLOR_WALL 15   // 白色
#define COLOR_TEXT 10   // 绿色

// 位置结构体
typedef struct {
    int x;
    int y;
    int active;  // 标记食物是否有效
} Position;

// 蛇结构体
typedef struct {
    Position pos[MAX_SNAKE_LENGTH];
    int length;
    int direction;
    int speed;
    int score;
    int is_accelerated;  // 新增：跟踪加速状态
    int base_score;  // 新增：记录每个食物的基础得分
} Snake;

// 全局变量
Snake snake;
Position foods[MAX_FOOD];  // 食物数组
int foodCount = 0;        // 当前食物数量
int highScore = 0;
int sleepTime = 200;
DWORD lastFoodTime = 0;   // 上次生成食物的时间
DWORD nextFoodInterval;   // 下次生成食物的时间间隔

// 函数声明
void initConsole(void);
void gotoxy(int x, int y);
void setColor(int color);
void printSnake(void);
void welcomeToGame(void);
void createMap(void);
void scoreAndTips(void);
void fileOut(void);
void initSnake(void);
void createFood(void);
int biteSelf(void);
void cantCrossWall(void);
void speedUp(void);
void speedDown(void);
void snakeMove(void);
void keyboardControl(void);
void gameOver(void);
void explanation(void);
void saveScore(int score);
void createSingleFood(void);
void initFoodSystem(void);
void updateFoodSystem(void);

// 初始化控制台
void initConsole(void) {
    // 设置控制台代码页为GBK
    system("chcp 936");
    // 设置控制台窗口大小
    system("mode con cols=120 lines=35");
    system("title 贪吃蛇大作战");
}

// 光标定位
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 设置颜色
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// 打印蛇标题
void printSnake(void) {
    system("cls");
    setColor(COLOR_SNAKE);
    printf("\n\n");
    printf("                  贪  吃  蛇  大  作  战\n");
    printf("                ========================\n");
    printf("                   Powered by C语言\n");
    printf("\n\n");
    setColor(COLOR_TEXT);
    printf("                  ┌──────────────────┐\n");
    printf("                  │    1. 开始游戏   │\n");
    printf("                  │    2. 游戏说明   │\n");
    printf("                  │    3. 退出游戏   │\n");
    printf("                  └──────────────────┘\n");
}

// 欢迎界面
void welcomeToGame(void) {
    char choice;
    while (1) {
        printSnake();
        choice = _getch();
        switch (choice) {
        case '1':
            system("cls");
            createMap();
            initSnake();
            initFoodSystem();  // 初始化食物系统
            keyboardControl();
            break;
        case '2':
            explanation();
            break;
        case '3':
            exit(0);
        default:
            continue;
        }
    }
}

// 创建地图
void createMap(void) {
    int i, j;
    setColor(COLOR_WALL);

    // 绘制上边框
    gotoxy(0, 0);
    printf("┌");
    for (i = 0; i < MAP_WIDTH - 2; i++)
        printf("─");
    printf("┐");

    // 绘制中间部分
    for (i = 1; i < MAP_HEIGHT - 1; i++) {
        gotoxy(0, i);
        printf("│");
        for (j = 0; j < MAP_WIDTH - 2; j++)
            printf(" ");
        printf("│");
    }

    // 绘制下边框
    gotoxy(0, MAP_HEIGHT - 1);
    printf("└");
    for (i = 0; i < MAP_WIDTH - 2; i++)
        printf("─");
    printf("┘");
}

// 显示分数和提示
void scoreAndTips(void) {
    gotoxy(MAP_WIDTH + 5, 3);
    printf("┌────────────────────┐");
    gotoxy(MAP_WIDTH + 5, 4);
    printf("│     游戏信息       │");
    gotoxy(MAP_WIDTH + 5, 5);
    printf("├────────────────────┤");
    gotoxy(MAP_WIDTH + 5, 6);
    printf("│ 当前得分: %-8d │", snake.score);
    gotoxy(MAP_WIDTH + 5, 7);
    printf("│ 历史最高: %-8d │", highScore);
    gotoxy(MAP_WIDTH + 5, 8);
    printf("├────────────────────┤");
    gotoxy(MAP_WIDTH + 5, 9);
    printf("│     操作说明       │");
    gotoxy(MAP_WIDTH + 5, 10);
    printf("│ ↑↓←→: 移动方向     │");
    gotoxy(MAP_WIDTH + 5, 11);
    printf("│ F1: 加速           │");
    gotoxy(MAP_WIDTH + 5, 12);
    printf("│ F2: 减速           │");
    gotoxy(MAP_WIDTH + 5, 13);
    printf("│ ESC: 退出          │");
    gotoxy(MAP_WIDTH + 5, 14);
    printf("├────────────────────┤");
    gotoxy(MAP_WIDTH + 5, 15);
    printf("│     游戏状态       │");
    gotoxy(MAP_WIDTH + 5, 16);
    printf("│ 蛇的长度: %-8d │", snake.length);
    gotoxy(MAP_WIDTH + 5, 17);
    printf("│ 当前速度: %-8d │", (400 - sleepTime) / 10);
    gotoxy(MAP_WIDTH + 5, 18);
    printf("└────────────────────┘");
}

// 读取最高分
void fileOut(void) {
    FILE* fp = fopen("save.txt", "r");
    if (fp) {
        fscanf(fp, "%d", &highScore);
        fclose(fp);
    }
}

// 初始化蛇
void initSnake(void) {
    int i;
    snake.length = INIT_SNAKE_LENGTH;
    snake.direction = RIGHT;
    snake.speed = 1;
    snake.score = 0;
    snake.is_accelerated = 0;
    snake.base_score = 10;  // 初始化基础得分为10

    for (i = 0; i < snake.length; i++) {
        snake.pos[i].x = MAP_WIDTH / 3 - i * 2;
        snake.pos[i].y = MAP_HEIGHT / 2;
        gotoxy(snake.pos[i].x, snake.pos[i].y);
        setColor(COLOR_SNAKE);
        if (i == 0) {
            printf("★");  // 蛇头
        }
        else {
            printf("○");  // 蛇身
        }
    }
}

// 创建单个食物
void createSingleFood(void) {
    int i, flag;
    Position newFood;

    do {
        flag = 0;
        newFood.x = (rand() % ((MAP_WIDTH - 4) / 2)) * 2 + 2;
        newFood.y = rand() % (MAP_HEIGHT - 2) + 1;

        // 检查是否与蛇身重叠
        for (i = 0; i < snake.length; i++) {
            if (newFood.x == snake.pos[i].x && newFood.y == snake.pos[i].y) {
                flag = 1;
                break;
            }
        }

        // 检查是否与其他食物重叠
        for (i = 0; i < MAX_FOOD; i++) {
            if (foods[i].active && newFood.x == foods[i].x && newFood.y == foods[i].y) {
                flag = 1;
                break;
            }
        }
    } while (flag);

    // 找到一个空闲的食物位置
    for (i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].active) {
            foods[i] = newFood;
            foods[i].active = 1;
            foodCount++;

            // 显示新食物
            gotoxy(foods[i].x, foods[i].y);
            setColor(COLOR_FOOD);
            printf("●");
            break;
        }
    }
}

// 初始化食物系统
void initFoodSystem(void) {
    int i;
    foodCount = 0;
    for (i = 0; i < MAX_FOOD; i++) {
        foods[i].active = 0;
    }
    lastFoodTime = GetTickCount();
    nextFoodInterval = rand() % (FOOD_INTERVAL_MAX - FOOD_INTERVAL_MIN + 1) + FOOD_INTERVAL_MIN;

    // 创建初始食物
    createSingleFood();
}

// 更新食物系统
void updateFoodSystem(void) {
    DWORD currentTime = GetTickCount();

    // 检查是否需要生成新食物
    if (currentTime - lastFoodTime >= nextFoodInterval) {
        if (foodCount < MAX_FOOD) {
            createSingleFood();
        }

        // 更新时间和间隔
        lastFoodTime = currentTime;
        nextFoodInterval = rand() % (FOOD_INTERVAL_MAX - FOOD_INTERVAL_MIN + 1) + FOOD_INTERVAL_MIN;
    }
}

// 修改原有的createFood函数
void createFood(void) {
    // 当食物数量低于最小值时，立即生成新食物
    if (foodCount < MIN_FOOD) {
        createSingleFood();
    }
}

// 检测是否咬到自己
int biteSelf(void) {
    int i;
    for (i = 1; i < snake.length; i++) {
        if (snake.pos[0].x == snake.pos[i].x && snake.pos[0].y == snake.pos[i].y)
            return 1;
    }
    return 0;
}

// 检测是否撞墙
void cantCrossWall(void) {
    if (snake.pos[0].x == 0 || snake.pos[0].x == MAP_WIDTH - 1 ||
        snake.pos[0].y == 0 || snake.pos[0].y == MAP_HEIGHT - 1) {
        gameOver();
    }
}

// 加速
void speedUp(void) {
    if (sleepTime >= 100) {  // 设置速度上限，最小延迟为100ms
        sleepTime -= 10;
        snake.is_accelerated = 1;
        if (snake.base_score < 20) {  // 设置得分上限为20
            snake.base_score += 1;  // 每次加速增加1分
        }
    }
}

// 减速
void speedDown(void) {
    if (sleepTime <= 320) {
        sleepTime += 30;
        snake.is_accelerated = 0;
        if (snake.base_score > 1) {  // 最低得分为1
            snake.base_score -= 1;  // 每次减速减少1分
        }
    }
}

// 蛇的移动
void snakeMove(void) {
    int i;
    gotoxy(snake.pos[snake.length - 1].x, snake.pos[snake.length - 1].y);
    printf(" ");

    for (i = snake.length - 1; i > 0; i--) {
        snake.pos[i] = snake.pos[i - 1];
    }

    switch (snake.direction) {
    case UP:
        snake.pos[0].y--;
        break;
    case DOWN:
        snake.pos[0].y++;
        break;
    case LEFT:
        snake.pos[0].x -= 2;
        break;
    case RIGHT:
        snake.pos[0].x += 2;
        break;
    }

    // 更新蛇的显示
    for (i = 0; i < snake.length; i++) {
        gotoxy(snake.pos[i].x, snake.pos[i].y);
        setColor(COLOR_SNAKE);
        if (i == 0) {
            printf("★");  // 蛇头
        }
        else {
            printf("○");  // 蛇身
        }
    }

    // 检查是否吃到食物
    for (i = 0; i < MAX_FOOD; i++) {
        if (foods[i].active && snake.pos[0].x == foods[i].x && snake.pos[0].y == foods[i].y) {
            snake.length++;
            snake.score += snake.base_score;
            foods[i].active = 0;
            foodCount--;
            // 只在食物数量不足时创建新食物
            if (foodCount < MIN_FOOD) {
                createFood();
            }
        }
    }

    // 更新食物系统
    updateFoodSystem();

    cantCrossWall();
    if (biteSelf()) {
        gameOver();
    }

    scoreAndTips();
    Sleep(sleepTime);
}

// 键盘控制
void keyboardControl(void) {
    while (1) {
        if (GetAsyncKeyState(VK_UP) && snake.direction != DOWN)
            snake.direction = UP;
        else if (GetAsyncKeyState(VK_DOWN) && snake.direction != UP)
            snake.direction = DOWN;
        else if (GetAsyncKeyState(VK_LEFT) && snake.direction != RIGHT)
            snake.direction = LEFT;
        else if (GetAsyncKeyState(VK_RIGHT) && snake.direction != LEFT)
            snake.direction = RIGHT;
        else if (GetAsyncKeyState(VK_F1))
            speedUp();
        else if (GetAsyncKeyState(VK_F2))
            speedDown();
        else if (GetAsyncKeyState(VK_ESCAPE))
            gameOver();

        snakeMove();
    }
}

// 游戏结束
void gameOver(void) {
    system("cls");
    setColor(COLOR_TEXT);
    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2);

    if (snake.score > highScore) {
        printf("恭喜你创造了新记录！得分：%d", snake.score);
        saveScore(snake.score);
    }
    else {
        printf("游戏结束！得分：%d（最高记录：%d）", snake.score, highScore);
    }

    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2 + 2);
    printf("1. 再玩一次");
    gotoxy(MAP_WIDTH / 3, MAP_HEIGHT / 2 + 3);
    printf("2. 退出游戏");

    while (1) {
        char choice = _getch();
        if (choice == '1') {
            system("cls");
            welcomeToGame();
        }
        else if (choice == '2') {
            exit(0);
        }
    }
}

// 游戏说明
void explanation(void) {
    system("cls");
    setColor(COLOR_TEXT);
    printf("\n\n");
    printf("  ┌──────────────────────────────────────┐\n");
    printf("  │          贪吃蛇游戏说明              │\n");
    printf("  ├──────────────────────────────────────┤\n");
    printf("  │                                      │\n");
    printf("  │    1. 使用方向键 ↑↓←→ 控制蛇的移动   │\n");
    printf("  │    2. 初始吃到食物可得10分           │\n");
    printf("  │    3. 撞到墙壁或咬到自己会游戏结束   │\n");
    printf("  │    4. 按F1加速：每次得分加1分        │\n");
    printf("  │    5. 按F2减速：每次得分减1分        │\n");
    printf("  │    6. 得分范围：最低1分最高20分      │\n");
    printf("  │    7. 按ESC键可以随时退出游戏        │\n");
    printf("  │                                      │\n");
    printf("  ├──────────────────────────────────────┤\n");
    printf("  │        按任意键返回主菜单...         │\n");
    printf("  └──────────────────────────────────────┘\n");

    // 清空键盘缓冲区
    while (_kbhit()) {
        _getch();
    }

    // 等待新的按键输入
    _getch();

    // 再次清空键盘缓冲区
    while (_kbhit()) {
        _getch();
    }

    // 确保清屏后再返回
    system("cls");
}

// 保存最高分
void saveScore(int score) {
    FILE* fp = fopen("save.txt", "w");
    if (fp) {
        fprintf(fp, "%d", score);
        fclose(fp);
        highScore = score;
    }
}

// 主函数
int main() {
    srand((unsigned)time(NULL));
    initConsole();
    fileOut();
    welcomeToGame();
    return 0;
}