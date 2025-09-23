#include <stdio.h>
#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <windows.h>
#include <float.h>
#include <ctime>
// 允许使用数学常量
#define _USE_MATH_DEFINES
// 高精度π常量，使用long double字面量以保留更多精度
const long double M_PI_L = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381L;
// 物理常量定义
#define WALL_X 50        // 墙壁位置
#define GROUND_Y 400     // 地面位置
#define BLOCK_WIDTH 40   // 物块宽度
#define BLOCK_HEIGHT 40  // 物块高度
#define EPSILON 1e-10    // 浮点比较阈值
#define HIGH_PRECISION_EPSILON 1e-18  // 高精度阈值（适配大质量比浮点误差）
#define MAX_CCD_ITERATIONS 1000000000    // 提升CCD最大迭代次数（支持高幂次碰撞计数，保险起见）
#define MAX_MASS_POWER 36             // 限制最大幂次（避免超出double精度极限）
#define LARGE_N_THRESHOLD 22           // 新增：大n阈值，当n > 此值时，使用解析近似（改进点：处理任意n）
// 物块结构体
typedef struct {
    long double mass;     // 改用long double提升质量存储精度
    long double x;        // 改用long double提升位置计算精度
    long double vx;       // 改用long double提升速度计算精度
    int color;            // 显示颜色
} Block;
// 全局变量
Block block1, block2;
long long collisionCount = 0;          // 改用long long避免碰撞次数溢出
int massRatioPower = 0;                // 10的幂次，表示质量比为10^massRatioPower
clock_t lastDisplayTime = 0;           // 上次显示信息的时间
const double DISPLAY_INTERVAL = 0.008;   // 调整显示间隔（0.008秒/次，避免高频刷新卡顿）
long long prevCollisionCount = 0;      // 上一帧的碰撞次数（同步为long long）
long double wsxnn;
// 函数声明
void initSimulation();
void updatePhysics();
void updatePhysicsWithCCD(long double dt);  // 参数改为long double
long double calculateAdaptiveTimestep();    // 返回值改为long double
void drawScene();
void handleWallCollision();
void handleBlockCollision();
void displayDynamicInfo();
void StartUI();
void UI();
void displayFinalResults();  // 新增：提取最终结果显示函数，便于大n时直接调用

void initSimulation() {
    // 计算质量比（1:10^massRatioPower），用powl提升精度
    long double massRatio = powl(10.0L, (long double)massRatioPower);
    // 初始化物块（所有物理量用long double）
    block1.mass = 1.0L;
    block1.x = (long double)WALL_X + (long double)BLOCK_WIDTH / 2.0L + 45.0L;  // 稍离墙壁
    block1.vx = 0.0L;  // 初始静止
    block1.color = RED;

    block2.mass = massRatio;
    block2.x = (long double)WALL_X + 350.0L;  // 初始位置
    block2.vx = wsxnn;  // 向左运动（速度保持适中，避免高幂次下计算爆炸；保持一致，无论n多少）
    block2.color = BLUE;

    collisionCount = 0;
}

long double calculateAdaptiveTimestep() {
    // 基础时间步长（随质量幂次动态减小，适配高幂次高频碰撞）
    long double dt = 1.0L / powl(10.0L, (long double)massRatioPower / 4.0L);
    // 速度约束：每步移动不超过物块1/4宽度（避免穿透）
    long double maxSpeed = fabsl(block1.vx) > fabsl(block2.vx) ? fabsl(block1.vx) : fabsl(block2.vx);
    if (maxSpeed > 0.0L) {
        long double speedBasedDt = (long double)BLOCK_WIDTH / (maxSpeed * 4.0L);
        dt = dt < speedBasedDt ? dt : speedBasedDt;
    }
    // 最小时间步长保护（避免过小导致性能问题）
    return dt < 1e-12L ? 1e-12L : dt;
}

// 迭代实现CCD（适配long double精度，处理一个dt内的所有碰撞）
void updatePhysicsWithCCD(long double dt) {
    long double remainingTime = dt;
    int iterations = 0;
    while (remainingTime > HIGH_PRECISION_EPSILON && iterations < MAX_CCD_ITERATIONS) {
        // 1. 计算到墙壁碰撞的时间（仅物块1可能撞墙）
        long double timeToWall = LDBL_MAX;
        if (block1.vx < -HIGH_PRECISION_EPSILON) {  // 避免微小负速度误判
            long double wallCollisionPos = (long double)WALL_X + (long double)BLOCK_WIDTH / 2.0L;
            timeToWall = (wallCollisionPos - block1.x) / block1.vx;
            timeToWall = timeToWall < 0.0L ? LDBL_MAX : timeToWall;  // 过滤过去时间
        }

        // 2. 计算两物块碰撞的时间（仅物块1速度>物块2时可能碰撞）
        long double timeToBlock = LDBL_MAX;
        if (block1.vx - block2.vx > HIGH_PRECISION_EPSILON) {  // 速度差为正才可能碰撞
            long double collisionGap = block2.x - block1.x - (long double)BLOCK_WIDTH;
            if (collisionGap < HIGH_PRECISION_EPSILON) {  // 已重叠，立即处理
                timeToBlock = 0.0L;
            }
            else {
                timeToBlock = collisionGap / (block1.vx - block2.vx);
            }
            timeToBlock = timeToBlock < 0.0L ? LDBL_MAX : timeToBlock;  // 过滤过去时间
        }

        // 3. 确定最近碰撞时间
        long double nextCollision = timeToWall < timeToBlock ? timeToWall : timeToBlock;

        // 4. 无碰撞：直接更新位置
        if (nextCollision > remainingTime + HIGH_PRECISION_EPSILON || nextCollision <= 0.0L) {
            block1.x += block1.vx * remainingTime;
            block2.x += block2.vx * remainingTime;
            break;
        }

        // 5. 有碰撞：先更新到碰撞位置，再处理碰撞
        if (nextCollision > HIGH_PRECISION_EPSILON) {  // 有时间差，先移动到碰撞前
            block1.x += block1.vx * nextCollision;
            block2.x += block2.vx * nextCollision;
        }

        // 6. 处理具体碰撞类型
        if (nextCollision == timeToWall) {
            // 墙壁碰撞（弹性碰撞，速度反向）
            handleWallCollision();
            collisionCount++;
        }
        else {
            // 物块碰撞（弹性碰撞公式，用long double计算）
            handleBlockCollision();
            collisionCount++;
        }

        // 7. 更新剩余时间和迭代次数
        remainingTime -= nextCollision;
        iterations++;
    }

    // 调试警告：超过最大迭代次数（高幂次时可适当提升MAX_CCD_ITERATIONS）
    if (iterations >= MAX_CCD_ITERATIONS) {
        printf("Warning: Reached maximum CCD iterations (remainingTime: %.2Le)\n", remainingTime);
    }
}

void updatePhysics() {
    long double dt = calculateAdaptiveTimestep();
    updatePhysicsWithCCD(dt);
}

void handleWallCollision() {
    // 位置校正：确保不穿墙
    long double wallLimit = (long double)WALL_X + (long double)BLOCK_WIDTH / 2.0L;
    if (block1.x < wallLimit - HIGH_PRECISION_EPSILON) {
        block1.x = wallLimit;
    }
    // 速度反向（弹性碰撞，无能量损失）
    block1.vx = -block1.vx;
}

void handleBlockCollision() {
    // 1. 位置校正：消除重叠（按质量比例分离，避免穿透）
    long double overlap = (block1.x + (long double)BLOCK_WIDTH / 2.0L) - (block2.x - (long double)BLOCK_WIDTH / 2.0L);
    if (overlap > HIGH_PRECISION_EPSILON) {
        long double totalMass = block1.mass + block2.mass;
        if (totalMass > HIGH_PRECISION_EPSILON) {  // 避免除以零（理论上mass>=1）
            block1.x -= overlap * (block2.mass / totalMass);
            block2.x += overlap * (block1.mass / totalMass);
        }
    }

    // 2. 弹性碰撞速度计算（用long double保留精度）
    long double m1 = block1.mass;
    long double m2 = block2.mass;
    long double v1 = block1.vx;
    long double v2 = block2.vx;
    long double totalMass = m1 + m2;

    if (totalMass > HIGH_PRECISION_EPSILON) {  // 避免除以零
        block1.vx = ((m1 - m2) * v1 + 2.0L * m2 * v2) / totalMass;
        block2.vx = (2.0L * m1 * v1 + (m2 - m1) * v2) / totalMass;
    }

    // 3. 速度阈值过滤：避免微小速度导致无限碰撞循环
    if (fabsl(block1.vx) < 1e-15L) block1.vx = 0.0L;
    if (fabsl(block2.vx) < 1e-15L) block2.vx = 0.0L;
}

void drawScene() {
    BeginBatchDraw();  // 开启批量绘图（避免闪烁）
    cleardevice();     // 清屏
    settextstyle(24, 0, "Consolas");  // 调整字体大小（适配更多信息显示）
    settextcolor(WHITE);              // 统一文字颜色（增强可读性）

    // 1. 绘制边界（墙壁+地面）
    // 墙壁（灰色矩形）
    setfillcolor(LIGHTGRAY);
    fillrectangle(WALL_X - 5, GROUND_Y - 100, WALL_X + 5, GROUND_Y);
    // 地面（白色直线）
    line(WALL_X, GROUND_Y, getwidth() - 50, GROUND_Y);

    // 2. 绘制物块（红色：block1，蓝色：block2）
    // 物块1
    setfillcolor(block1.color);
    fillrectangle(
        (int)(block1.x - (long double)BLOCK_WIDTH / 2.0L),
        GROUND_Y - BLOCK_HEIGHT,
        (int)(block1.x + (long double)BLOCK_WIDTH / 2.0L),
        GROUND_Y
    );
    // 物块2
    setfillcolor(block2.color);
    fillrectangle(
        (int)(block2.x - (long double)BLOCK_WIDTH / 2.0L),
        GROUND_Y - BLOCK_HEIGHT,
        (int)(block2.x + (long double)BLOCK_WIDTH / 2.0L),
        GROUND_Y
    );

    // 3. 绘制固定信息（质量比）
    char massBuf[64];
    sprintf(massBuf, "质量比: 1 : 10^%d", massRatioPower);
    outtextxy(50, 50, massBuf);

    // 4. 绘制动态信息（碰撞次数、理论值、误差）
    clock_t currentTime = clock();
    double elapsed = (double)(currentTime - lastDisplayTime) / CLOCKS_PER_SEC;
    if (elapsed >= DISPLAY_INTERVAL || collisionCount != prevCollisionCount) {
        displayDynamicInfo();
        lastDisplayTime = currentTime;
        prevCollisionCount = collisionCount;
    }

    EndBatchDraw();  // 提交绘图（确保显示生效）
}

void displayDynamicInfo() {
    BeginBatchDraw();
    char infoBuf[256];  // 扩大缓冲区（适配长数字显示）
    long double theoreticalValue = M_PI_L * powl(10.0L, (long double)massRatioPower / 2.0L);  // 统一计算理论值（π * 10^{n/2}）

    long long collisionTheoretical = (long long)floorl(theoreticalValue);  // 理论值取整

    // 1. 显示碰撞次数（支持long long）
    sprintf(infoBuf, "碰撞次数: %lld", collisionCount);
    outtextxy(50, 80, infoBuf);

    // 2. 显示理论值（适配高幂次的大数字）
    sprintf(infoBuf, "理论值(π*10^(%d/2)): %.8Lf",
        massRatioPower, theoreticalValue);
    sprintf(infoBuf, "取整: %lld",
        collisionTheoretical);
    outtextxy(50, 110, infoBuf);

    // 3. 显示误差（避免理论值为0的情况）
    if (collisionTheoretical != 0) {
        long double errorPercent = fabsl((long double)collisionCount - theoreticalValue) / theoreticalValue * 100.0L;
        sprintf(infoBuf, "误差(小概率不计，10^-4以下): %.2Lf%%", errorPercent);
    }
    else {
        sprintf(infoBuf, "误差: %lld (N/A，理论值为0)", collisionCount - collisionTheoretical);
    }
    outtextxy(50, 140, infoBuf);

    // 4. 碰撞完成提示（判断条件适配long double）
    if (block1.vx >= -HIGH_PRECISION_EPSILON &&
        block2.vx >= block1.vx - HIGH_PRECISION_EPSILON &&
        (block2.x - block1.x) > (long double)BLOCK_WIDTH * 1.1L) {
        outtextxy(50, 170, "所有碰撞已完成!");
    }
    EndBatchDraw();
}

// 新增：提取最终结果显示函数（用于模拟结束或大n直接显示）
void displayFinalResults() {
    BeginBatchDraw();
    cleardevice();
    settextstyle(24, 0, "Consolas");
    settextcolor(WHITE);

    // 1. 固定信息
    char massBuf[64];
    sprintf(massBuf, "质量比: 1 : 10^%d", massRatioPower);
    outtextxy(50, 50, massBuf);

    // 2. 最终碰撞次数
    char finalBuf[256];
    sprintf(finalBuf, "最终碰撞次数: %lld", collisionCount);
    outtextxy(50, 80, finalBuf);

    // 3. 理论值与误差
    long double theoreticalValue = M_PI_L * powl(10.0L, (long double)massRatioPower / 2.0L);
    long long collisionTheoretical = (long long)floorl(theoreticalValue);
    sprintf(finalBuf, "理论值(π*10^(%d/2)): %.8Lf",
        massRatioPower, theoreticalValue);
    sprintf(finalBuf, "取整: %lld",collisionTheoretical);
    outtextxy(50, 110, finalBuf);

    // 4. 误差计算
    if (collisionTheoretical != 0) {
        long double errorPercent = fabsl((long double)collisionCount - theoreticalValue) / theoreticalValue * 100.0L;
        sprintf(finalBuf, "误差(小概率不计，10^-4以下):%.2Lf%%", errorPercent);
    }
    else {
        sprintf(finalBuf, "误差: %lld (N/A，理论值为0)", collisionCount - collisionTheoretical);
    }
    outtextxy(50, 140, finalBuf);

    // 5. 结束提示（大n时添加近似说明）
    if (massRatioPower > LARGE_N_THRESHOLD) {
        outtextxy(50, 170, "大n使用解析近似! 按任意键退出...");
    }
    else {
        outtextxy(50, 170, "所有碰撞已结束! 按任意键退出...");
    }
    EndBatchDraw();
}

// 启动界面（5秒倒计时）
void StartUI() {
    cleardevice();
    BeginBatchDraw();
    settextstyle(60, 0, "Consolas");
    const char* title = "物理引擎演示：碰撞计数与π的关系";
    outtextxy((getwidth() - textwidth(title)) / 2, 50, title);

    settextstyle(30, 0, "Consolas");
    const char* instructions[] = {
        "物块1（红色）质量为1，初始静止",
        "物块2（蓝色）质量为10^%d，初始向左运动",
        "物块1与墙壁弹性碰撞，物块间弹性碰撞",
        "观察碰撞次数与质量比的关系"
    };
    // 填充质量幂次到说明文字
    char instBuf[128];
    for (int i = 0; i < 4; i++) {
        if (i == 1) {
            sprintf(instBuf, instructions[i], massRatioPower);
            outtextxy(50, 150 + i * 30, instBuf);
        }
        else {
            outtextxy(50, 150 + i * 30, instructions[i]);
        }
    }

    // 倒计时显示
    for (int t = 5; t > 0; t--) {
        char timerBuf[32];
        sprintf(timerBuf, "%ds后开始...", t);
        outtextxy(50, 350, timerBuf);
        EndBatchDraw();
        Sleep(1000);
        cleardevice();
        // 重绘标题和说明（避免倒计时闪烁）
        outtextxy((getwidth() - textwidth(title)) / 2, 50, title);
        for (int i = 0; i < 4; i++) {
            if (i == 1) {
                sprintf(instBuf, instructions[i], massRatioPower);
                outtextxy(50, 150 + i * 30, instBuf);
            }
            else {
                outtextxy(50, 150 + i * 30, instructions[i]);
            }
        }
    }
    EndBatchDraw();
}

// 终端操作提示界面
void UI() {
    cleardevice();
    BeginBatchDraw();
    settextstyle(60, 0, "Consolas");
    const char* title = "物理引擎演示：碰撞计数与π的关系";
    outtextxy((getwidth() - textwidth(title)) / 2, 50, title);

    settextstyle(35, 0, "Consolas");
    const char* tip = "请在终端中输入质量比的幂次n(n为偶数才有规律) ，质量比为1:10^n";
    for (int i = 1; i <= 5; i++) {
        outtextxy((getwidth() - textwidth(tip)) / 2, 240 + 50 * i, tip);
    }
    EndBatchDraw();
}

int main() {
    // 初始化图形窗口（扩大宽度至1100，适配更多文字显示）
    initgraph(1100, 600);
    setbkcolor(BLACK);  // 黑色背景（增强文字对比度）

    // 终端交互：输入质量幂次（增加合法性校验）
    printf("=========================================\n");
    printf("      物理引擎演示：碰撞计数与π的关系     \n");
    printf("=========================================\n");
    printf("规则：\n");
    printf("  - 物块1（红）质量=1，初始静止\n");
    printf("  - 物块2（蓝）质量=10^n，初始向左运动\n");
    printf("  - 碰撞次数理论值≈π×10^(n/2)(n为偶数才有规律)\n");
    printf("=========================================\n");

    UI();  // 显示图形界面提示
    while (1) {
        printf("请输入质量比的幂次n(n为偶数才有规律)：");
        if (scanf_s("%d", &massRatioPower) != 1) {
            // 输入非数字时清空缓冲区
            while (getchar() != '\n');
            printf("错误：请输入整数！\n");
            continue;
        }
        // 限制幂次范围（1≤n≤22，避免超出long double精度）
        if (massRatioPower < 1 || massRatioPower > MAX_MASS_POWER) {
            printf("错误：n必须在1~%d之间！\n", MAX_MASS_POWER);
            continue;
        }
        break;  // 输入合法，退出循环
    }
    if (massRatioPower >= 21) {
        wsxnn = -(massRatioPower + 1) * 90000000000.0; // 根据质量幂次调整初始速度，避免高幂次下计算爆炸
    }
    if (massRatioPower >= 16) {
        wsxnn = -(massRatioPower + 1) * 4.625; // 根据质量幂次调整初始速度，避免高幂次下计算爆炸
    }
    else if (massRatioPower >= 12) {
		wsxnn = -3.0; // 适中速度，避免过快导致穿透
    }
    else{
        if(massRatioPower >= 6)
        wsxnn = -0.345; // 适中速度，避免过快导致穿透
        else
			wsxnn = -0.021; // 适中速度，避免过快导致穿透
	}
    bool isLargeN = (massRatioPower > LARGE_N_THRESHOLD);  // 新增：判断是否大n

    if (!isLargeN) {
        // 小n：正常模拟
        initSimulation();
        StartUI();
        lastDisplayTime = clock();
        prevCollisionCount = collisionCount;

        // 主循环（模拟运行，使用自适应dt推进模拟，按实时控制绘制频率）
        clock_t lastFrameTime = clock();
        bool simulationComplete = false;
        while (!kbhit() && !simulationComplete) {
            updatePhysics();  // 使用自适应dt推进一小步模拟（确保精度和性能）

            // 检查模拟是否完成（两物块均向右且无重叠）
            if (block1.vx >= -HIGH_PRECISION_EPSILON &&
                block2.vx >= block1.vx - HIGH_PRECISION_EPSILON &&
                (block2.x - block1.x) > (long double)BLOCK_WIDTH * 1.1L) {
                simulationComplete = true;
            }

            // 按实时绘制（每0.008s ~120fps，避免高n下绘制过频）
            clock_t currentTime = clock();
            double elapsedSinceLastFrame = (double)(currentTime - lastFrameTime) / CLOCKS_PER_SEC;
            if (elapsedSinceLastFrame >= 0.008) {
                drawScene();
                lastFrameTime = currentTime;
            }
        }
    }
    else {
        // 大n：使用解析近似计算碰撞次数（改进点：支持任意n，无需模拟）
        long double theoreticalValue = M_PI_L * powl(10.0L, (long double)massRatioPower / 2.0L);
        collisionCount = (long long)floorl(theoreticalValue);
        // 无需初始化块或运行循环，直接准备显示
    }

    // 无论模拟或近似，都显示最终结果
    displayFinalResults();

    // 等待用户按键退出
    _getch();
    closegraph();
    return 0;
}