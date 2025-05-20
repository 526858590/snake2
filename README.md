# 贪吃蛇大作战 (Snake Game)

一个基于C语言开发的经典贪吃蛇游戏，具有现代化的界面设计和丰富的游戏特性。本游戏采用Windows控制台实现，通过精心设计的界面和流畅的游戏体验，为玩家带来经典与现代完美结合的游戏体验。

## 项目特点

- 美观的图形界面，使用Windows控制台实现
  - 采用Unicode字符绘制游戏元素，支持中文显示
  - 使用不同颜色区分游戏元素，提升视觉体验
  - 精心设计的游戏边框和信息面板

- 动态食物生成系统
  - 支持多个食物同时存在
  - 随机生成食物位置，避免重叠
  - 动态调整食物生成间隔，增加游戏趣味性

- 可调节的游戏速度
  - 支持实时速度调节
  - 速度变化影响得分系统
  - 平滑的速度过渡效果

- 分数系统与历史最高分记录
  - 基于速度的动态得分机制
  - 持久化的最高分记录
  - 实时显示游戏状态和得分

- 完整的游戏说明和操作指南
  - 清晰的游戏规则说明
  - 直观的操作提示
  - 详细的游戏状态显示

## 技术实现

### 核心数据结构

1. **位置结构体 (Position)**
   ```c
   typedef struct {
       int x;      // x坐标
       int y;      // y坐标
       int active; // 标记是否激活（用于食物系统）
   } Position;
   ```
   - 用于表示蛇身节点和食物的位置
   - `active` 标记用于食物系统，表示食物是否有效
   - 坐标系统采用二维平面坐标系
   - 使用整数坐标确保精确的位置控制
   - 支持游戏元素的精确定位和碰撞检测

2. **蛇结构体 (Snake)**
   ```c
   typedef struct {
       Position pos[MAX_SNAKE_LENGTH];  // 蛇身位置数组
       int length;                      // 当前长度
       int direction;                   // 移动方向
       int speed;                       // 移动速度
       int score;                       // 当前得分
       int is_accelerated;              // 是否处于加速状态
       int base_score;                  // 基础得分值
   } Snake;
   ```
   - 存储蛇的所有属性，包括位置、长度、方向等
   - 实现了加速状态跟踪和基础得分系统
   - 使用数组存储蛇身的每个节点位置
   - 支持动态长度变化
   - 维护游戏状态和得分信息

### 主要功能模块

1. **游戏初始化系统**
   ```c
   // 初始化控制台
   void initConsole(void) {
       system("chcp 936");  // 设置控制台代码页为GBK，支持中文显示
       system("mode con cols=120 lines=35");  // 设置控制台窗口大小
       system("title 贪吃蛇大作战");  // 设置窗口标题
   }

   // 光标定位函数
   void gotoxy(int x, int y) {
       COORD coord;
       coord.X = x;
       coord.Y = y;
       SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
   }

   // 设置控制台文字颜色
   void setColor(int color) {
       SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
   }

   // 初始化蛇
   void initSnake(void) {
       int i;
       // 初始化蛇的基本属性
       snake.length = INIT_SNAKE_LENGTH;  // 设置初始长度
       snake.direction = RIGHT;           // 设置初始方向
       snake.speed = 1;                   // 设置初始速度
       snake.score = 0;                   // 初始化分数
       snake.is_accelerated = 0;          // 初始化加速状态
       snake.base_score = 10;             // 设置基础得分

       // 初始化蛇身位置
       for (i = 0; i < snake.length; i++) {
           snake.pos[i].x = MAP_WIDTH / 3 - i * 2;  // 水平位置递减
           snake.pos[i].y = MAP_HEIGHT / 2;         // 垂直位置居中
           gotoxy(snake.pos[i].x, snake.pos[i].y);
           setColor(COLOR_SNAKE);
           if (i == 0) {
               printf("★");  // 蛇头使用星形符号
           } else {
               printf("○");  // 蛇身使用圆形符号
           }
       }
   }
   ```
   初始化系统负责：
   - 设置控制台环境，包括窗口大小和编码
   - 初始化游戏界面和显示参数
   - 创建初始蛇身和设置基本属性
   - 准备游戏运行环境

2. **食物系统**
   ```c
   // 创建单个食物
   void createSingleFood(void) {
       int i, flag;
       Position newFood;

       do {
           flag = 0;
           // 生成随机位置，确保在有效范围内
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
       } while (flag);  // 如果位置无效，重新生成

       // 找到一个空闲的食物位置并放置食物
       for (i = 0; i < MAX_FOOD; i++) {
           if (!foods[i].active) {
               foods[i] = newFood;
               foods[i].active = 1;
               foodCount++;
               gotoxy(foods[i].x, foods[i].y);
               setColor(COLOR_FOOD);
               printf("●");  // 使用实心圆表示食物
               break;
           }
       }
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
   ```
   食物系统特点：
   - 支持多个食物同时存在
   - 智能的位置生成算法，避免重叠
   - 动态的食物生成间隔
   - 食物状态管理和显示更新

3. **移动控制系统**
   ```c
   // 蛇的移动
   void snakeMove(void) {
       int i;
       // 清除蛇尾
       gotoxy(snake.pos[snake.length - 1].x, snake.pos[snake.length - 1].y);
       printf(" ");

       // 移动蛇身
       for (i = snake.length - 1; i > 0; i--) {
           snake.pos[i] = snake.pos[i - 1];
       }

       // 根据方向移动蛇头
       switch (snake.direction) {
           case UP:    snake.pos[0].y--; break;
           case DOWN:  snake.pos[0].y++; break;
           case LEFT:  snake.pos[0].x -= 2; break;
           case RIGHT: snake.pos[0].x += 2; break;
       }

       // 更新蛇的显示
       for (i = 0; i < snake.length; i++) {
           gotoxy(snake.pos[i].x, snake.pos[i].y);
           setColor(COLOR_SNAKE);
           if (i == 0) {
               printf("★");  // 蛇头
           } else {
               printf("○");  // 蛇身
           }
       }

       // 检查是否吃到食物
       for (i = 0; i < MAX_FOOD; i++) {
           if (foods[i].active && snake.pos[0].x == foods[i].x && snake.pos[0].y == foods[i].y) {
               snake.length++;  // 增加长度
               snake.score += snake.base_score;  // 增加分数
               foods[i].active = 0;  // 移除食物
               foodCount--;
               if (foodCount < MIN_FOOD) {
                   createFood();  // 如果食物数量不足，创建新食物
               }
           }
       }

       // 更新游戏状态
       updateFoodSystem();
       cantCrossWall();  // 检查是否撞墙
       if (biteSelf()) {  // 检查是否咬到自己
           gameOver();
       }

       scoreAndTips();  // 更新分数显示
       Sleep(sleepTime);  // 控制游戏速度
   }

   // 键盘控制
   void keyboardControl(void) {
       while (1) {
           // 检测方向键
           if (GetAsyncKeyState(VK_UP) && snake.direction != DOWN)
               snake.direction = UP;
           else if (GetAsyncKeyState(VK_DOWN) && snake.direction != UP)
               snake.direction = DOWN;
           else if (GetAsyncKeyState(VK_LEFT) && snake.direction != RIGHT)
               snake.direction = LEFT;
           else if (GetAsyncKeyState(VK_RIGHT) && snake.direction != LEFT)
               snake.direction = RIGHT;
           // 检测功能键
           else if (GetAsyncKeyState(VK_F1))
               speedUp();
           else if (GetAsyncKeyState(VK_F2))
               speedDown();
           else if (GetAsyncKeyState(VK_ESCAPE))
               gameOver();

           snakeMove();
       }
   }
   ```
   移动控制系统特点：
   - 流畅的移动效果
   - 精确的碰撞检测
   - 实时的键盘响应
   - 智能的方向控制

4. **游戏机制**
   ```c
   // 加速
   void speedUp(void) {
       if (sleepTime >= 100) {  // 设置速度上限，最小延迟为100ms
           sleepTime -= 10;     // 减少延迟时间，提高速度
           snake.is_accelerated = 1;  // 标记加速状态
           if (snake.base_score < 20) {  // 设置得分上限为20
               snake.base_score += 1;  // 每次加速增加1分
           }
       }
   }

   // 减速
   void speedDown(void) {
       if (sleepTime <= 320) {  // 设置速度下限，最大延迟为320ms
           sleepTime += 30;     // 增加延迟时间，降低速度
           snake.is_accelerated = 0;  // 取消加速状态
           if (snake.base_score > 1) {  // 最低得分为1
               snake.base_score -= 1;  // 每次减速减少1分
           }
       }
   }

   // 检测是否咬到自己
   int biteSelf(void) {
       int i;
       for (i = 1; i < snake.length; i++) {
           if (snake.pos[0].x == snake.pos[i].x && snake.pos[0].y == snake.pos[i].y)
               return 1;  // 如果蛇头与任何身体部分重叠，返回1
       }
       return 0;  // 否则返回0
   }

   // 检测是否撞墙
   void cantCrossWall(void) {
       if (snake.pos[0].x == 0 || snake.pos[0].x == MAP_WIDTH - 1 ||
           snake.pos[0].y == 0 || snake.pos[0].y == MAP_HEIGHT - 1) {
           gameOver();  // 如果蛇头碰到边界，游戏结束
       }
   }
   ```
   游戏机制特点：
   - 动态的速度调节系统
   - 基于速度的得分机制
   - 精确的碰撞检测
   - 合理的游戏规则设计

5. **分数系统**
   ```c
   // 保存最高分
   void saveScore(int score) {
       FILE* fp = fopen("save.txt", "w");
       if (fp) {
           fprintf(fp, "%d", score);  // 将分数写入文件
           fclose(fp);
           highScore = score;  // 更新最高分
       }
   }

   // 读取最高分
   void fileOut(void) {
       FILE* fp = fopen("save.txt", "r");
       if (fp) {
           fscanf(fp, "%d", &highScore);  // 从文件读取最高分
           fclose(fp);
       }
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
   ```
   分数系统特点：
   - 持久化的分数记录
   - 实时分数显示
   - 完整的游戏状态信息
   - 清晰的操作提示

### 游戏参数配置

```c
#define MAP_WIDTH 80        // 地图宽度
#define MAP_HEIGHT 25       // 地图高度
#define INIT_SNAKE_LENGTH 5 // 初始蛇长度
#define MAX_SNAKE_LENGTH 100// 最大蛇长度
#define MAX_FOOD 5         // 最大食物数量
#define MIN_FOOD 1         // 最小食物数量
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
```

## 操作说明

- 方向键：控制蛇的移动方向
  - ↑：向上移动
  - ↓：向下移动
  - ←：向左移动
  - →：向右移动

- 功能键：
  - F1：加速（同时增加得分）
  - F2：减速（同时减少得分）
  - ESC：退出游戏

## 得分规则

- 基础得分：10分
- 加速状态：每次得分+1（最高20分）
- 减速状态：每次得分-1（最低1分）
- 得分计算：
  - 初始状态：每吃一个食物得10分
  - 加速状态：每按一次F1，基础得分+1
  - 减速状态：每按一次F2，基础得分-1

## 开发环境

- 操作系统：Windows
- 编译器：支持C语言的编译器
- 依赖库：
  - windows.h：Windows API支持
  - conio.h：控制台输入输出支持

## 编译运行

1. 确保系统已安装C语言编译器
2. 编译源代码：
   ```bash
   gcc snake_vs.c -o snake_game
   ```
3. 运行游戏：
   ```bash
   ./snake_game
   ```

## 注意事项

- 游戏需要Windows系统环境
- 控制台窗口大小已优化为120x35
- 游戏数据保存在save.txt文件中
- 确保控制台支持中文显示
- 建议使用等宽字体以获得最佳显示效果

## 未来改进方向

1. 添加多人对战模式
   - 支持本地双人对战
   - 实现网络对战功能
   - 添加排行榜系统

2. 实现更多类型的食物
   - 特殊食物效果
   - 随机事件系统
   - 道具系统

3. 优化游戏界面
   - 添加动画效果
   - 改进视觉效果
   - 支持自定义主题

4. 添加游戏难度选择
   - 多个难度等级
   - 自定义游戏参数
   - 成就系统
