
# 🎓 计算机图形学实验报告

> **学号**：
> **姓名**：
> **日期**：2025年12月26日

---

# 📑 目录

1. [期中大作业 - 俄罗斯方块](#一期中大作业---俄罗斯方块)
2. [期末大作业 - 御剑飞行虚拟场景](#二期末大作业---御剑飞行虚拟场景)
3. [核心技术总结](#三核心技术总结)
4. [编译运行指南](#四编译运行指南)

---

# 一、期中大作业 - 俄罗斯方块

## 1.1 项目概述

使用 **OpenGL 3.3+** 实现经典俄罗斯方块游戏，包含完整游戏逻辑、渲染和交互。

**项目路径**：`D:\learn\vcc\code\`

## 1.2 功能清单

| 功能      | 说明                | 状态 |
| --------- | ------------------- | :--: |
| 7种方块   | L、O、I、Z、S、J、T |  ✅  |
| 方块旋转  | 4个方向             |  ✅  |
| 方块移动  | 左右下              |  ✅  |
| 加速下落  | 空格键              |  ✅  |
| 消行计分  | 满行消除            |  ✅  |
| 难度递增  | 速度加快            |  ✅  |
| 暂停/继续 | P键                 |  ✅  |
| 重新开始  | R键                 |  ✅  |

## 1.3 核心代码与技术讲解

### 1.3.1 方块数据结构设计

```cpp
// 7种方块的4个旋转状态，每个方块由4个格子组成
// 使用相对坐标表示，(0,0)为方块中心
glm::vec2 AllRotationsLshape[7][4][4] = {
    {   // 1. L形方块
        { glm::vec2(0, 0), glm::vec2(-1, 0), glm::vec2(1, 0), glm::vec2(-1, -1) },  // 状态0
        { glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(1, -1) },   // 状态1（顺时针90°）
        { glm::vec2(1, 1), glm::vec2(-1, 0), glm::vec2(0, 0), glm::vec2(1, 0) },    // 状态2（180°）
        { glm::vec2(-1, 1), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0, -1) }    // 状态3（270°）
    },
    {   // 2. O形方块（正方形，旋转不变）
        { glm::vec2(0, 0), glm::vec2(-1, 0), glm::vec2(0, -1), glm::vec2(-1, -1) },
        { glm::vec2(0, 0), glm::vec2(-1, 0), glm::vec2(0, -1), glm::vec2(-1, -1) },
        { glm::vec2(0, 0), glm::vec2(-1, 0), glm::vec2(0, -1), glm::vec2(-1, -1) },
        { glm::vec2(0, 0), glm::vec2(-1, 0), glm::vec2(0, -1), glm::vec2(-1, -1) }
    },
    {   // 3. I形方块（长条）
        { glm::vec2(1, 0), glm::vec2(0, 0), glm::vec2(-1, 0), glm::vec2(-2, 0) },   // 水平
        { glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(0, -2) },   // 垂直
        // ... 
    },
    // 4. Z形、5. S形、6. J形、7. T形 类似定义...
};

// 9种随机颜色
glm::vec4 Orange = glm::vec4(1.0, 0.5, 0.0, 1.0);
glm::vec4 White = glm::vec4(1.0, 1.0, 1.0, 1.0);
glm::vec4 Pink = glm::vec4(1.0, 0.75, 0.8, 1.0);
// ...
const glm::vec4 TileColors[9] = { Orange, White, Pink, Red, Blue, Green, Yellow, Purple, Skyblue };
```

**技术要点**：使用相对坐标系统，每个方块以中心点为原点，4个格子的位置用偏移量表示。旋转时只需切换到下一个状态数组即可。

---

# 1.3.2 VAO/VBO 初始化与管理

```cpp
GLuint Vao[3], Vbo[6];  // 3个VAO，6个VBO

void Init() {
    // ========== VAO[0]: 棋盘网格线 ==========
    glm::vec4 GridPoints[BoardLineCnt * 2];  // 网格线顶点
    glm::vec4 GridColors[BoardLineCnt * 2];  // 网格线颜色
  
    // 绘制纵向网格线
    for (int i = 0; i < BoardWidth + 1; i++) {
        GridPoints[2*i] = glm::vec4(TileSize + TileSize*i, TileSize, 0, 1);
        GridPoints[2*i + 1] = glm::vec4(TileSize + TileSize*i, (BoardHeight+1)*TileSize, 0, 1);
    }
    // 绘制横向网格线
    for (int i = 0; i < BoardHeight + 1; i++) {
        GridPoints[2*(BoardWidth+1) + 2*i] = glm::vec4(TileSize, TileSize + TileSize*i, 0, 1);
        GridPoints[2*(BoardWidth+1) + 2*i + 1] = glm::vec4((BoardWidth+1)*TileSize, TileSize + TileSize*i, 0, 1);
    }
    for (int i = 0; i < BoardLineCnt * 2; i++) 
        GridColors[i] = White;  // 白色网格线
  
    glGenVertexArrays(3, &Vao[0]);
    glBindVertexArray(Vao[0]);
    glGenBuffers(2, Vbo);
  
    // VBO[0]: 网格线顶点位置
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, (BoardLineCnt*2)*sizeof(glm::vec4), GridPoints, GL_STATIC_DRAW);
    glVertexAttribPointer(VPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VPosition);
  
    // VBO[1]: 网格线颜色
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, (BoardLineCnt*2)*sizeof(glm::vec4), GridColors, GL_STATIC_DRAW);
    glVertexAttribPointer(VColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VColor);
  
    // ========== VAO[1]: 棋盘格子（已放置的方块） ==========
    glBindVertexArray(Vao[1]);
    glGenBuffers(2, &Vbo[2]);
  
    // 每个格子6个顶点（2个三角形）
    glm::vec4 BoardPoints[PointsCnt];
    for (int i = 0; i < BoardHeight; i++) {
        for (int j = 0; j < BoardWidth; j++) {
            glm::vec4 p1 = glm::vec4(TileSize + j*TileSize, TileSize + i*TileSize, 0.5, 1);
            glm::vec4 p2 = glm::vec4(TileSize + j*TileSize, TileSize*2 + i*TileSize, 0.5, 1);
            glm::vec4 p3 = glm::vec4(TileSize*2 + j*TileSize, TileSize + i*TileSize, 0.5, 1);
            glm::vec4 p4 = glm::vec4(TileSize*2 + j*TileSize, TileSize*2 + i*TileSize, 0.5, 1);
            // 两个三角形：p1-p2-p3 和 p2-p3-p4
            BoardPoints[6*(BoardWidth*i + j) + 0] = p1;
            BoardPoints[6*(BoardWidth*i + j) + 1] = p2;
            BoardPoints[6*(BoardWidth*i + j) + 2] = p3;
            BoardPoints[6*(BoardWidth*i + j) + 3] = p2;
            BoardPoints[6*(BoardWidth*i + j) + 4] = p3;
            BoardPoints[6*(BoardWidth*i + j) + 5] = p4;
        }
    }
    // VBO[2]: 棋盘格子顶点 (STATIC)
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, PointsCnt*sizeof(glm::vec4), BoardPoints, GL_STATIC_DRAW);
  
    // VBO[3]: 棋盘格子颜色 (DYNAMIC - 需要动态更新)
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, PointsCnt*sizeof(glm::vec4), BoardColor, GL_DYNAMIC_DRAW);
  
    // ========== VAO[2]: 当前下落方块 ==========
    glBindVertexArray(Vao[2]);
    glGenBuffers(2, &Vbo[4]);
  
    // VBO[4]: 当前方块顶点 (DYNAMIC)
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);  // 4格子×6顶点
  
    // VBO[5]: 当前方块颜色 (DYNAMIC)
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[5]);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
}
```

**技术要点**：

- **VAO[0]**：静态网格线，使用 `GL_STATIC_DRAW`
- **VAO[1]**：棋盘格子，顶点静态但颜色动态（`GL_DYNAMIC_DRAW`）
- **VAO[2]**：当前方块，位置和颜色都是动态的

---

### 1.3.3 方块动态更新（VBO SubData）

```cpp
// 当前方块移动或旋转时，更新VBO数据
void UpdateTile() {
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[4]);
  
    for (int i = 0; i < 4; i++) {
        // 计算格子在屏幕上的实际坐标
        GLfloat x = TileCoordinate.x + Tile[i].x;
        GLfloat y = TileCoordinate.y + Tile[i].y;
      
        // 计算4个顶点
        glm::vec4 p1 = glm::vec4(TileSize + x*TileSize, TileSize + y*TileSize, 0.4, 1);
        glm::vec4 p2 = glm::vec4(TileSize + x*TileSize, TileSize*2 + y*TileSize, 0.4, 1);
        glm::vec4 p3 = glm::vec4(TileSize*2 + x*TileSize, TileSize + y*TileSize, 0.4, 1);
        glm::vec4 p4 = glm::vec4(TileSize*2 + x*TileSize, TileSize*2 + y*TileSize, 0.4, 1);
      
        // 2个三角形 = 6个顶点
        glm::vec4 NewPoints[6] = { p1, p2, p3, p2, p3, p4 };
      
        // 使用glBufferSubData局部更新，避免重新分配整个缓冲区
        glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(glm::vec4), 6*sizeof(glm::vec4), NewPoints);
    }
    glBindVertexArray(0);
}
```

**技术要点**：`glBufferSubData` 只更新缓冲区的指定区域，比 `glBufferData` 高效得多。

---

### 1.3.4 碰撞检测

```cpp
// 检查坐标是否有效（未越界且未被占据）
bool CheckValid(glm::vec2 Coordinate) {
    // 边界检测
    if (Coordinate.x < 0 || Coordinate.x >= BoardWidth) return false;
    if (Coordinate.y < 0 || Coordinate.y >= BoardHeight) return false;
    // 占据检测
    if (BoardOccupied[(int)Coordinate.x][(int)Coordinate.y]) return false;
    return true;
}

// 移动方块
bool MoveTile(glm::vec2 Direction) {
    glm::vec2 NewTileCoordinate[4];
    for (int i = 0; i < 4; i++)
        NewTileCoordinate[i] = Tile[i] + TileCoordinate + Direction;
  
    // 检查移动后所有4个格子是否合法
    if (CheckValid(NewTileCoordinate[0]) && CheckValid(NewTileCoordinate[1])
        && CheckValid(NewTileCoordinate[2]) && CheckValid(NewTileCoordinate[3])) {
        TileCoordinate.x += Direction.x;
        TileCoordinate.y += Direction.y;
        UpdateTile();
        return true;
    }
    return false;
}
```

---

### 1.3.5 旋转算法

```cpp
void Rotate() {
    int NextRotation = (Rotation + 1) % 4;  // 计算下一个旋转状态
  
    // 检查旋转后的4个格子是否都在有效位置
    if (CheckValid(AllRotationsLshape[TileType][NextRotation][0] + TileCoordinate)
        && CheckValid(AllRotationsLshape[TileType][NextRotation][1] + TileCoordinate)
        && CheckValid(AllRotationsLshape[TileType][NextRotation][2] + TileCoordinate)
        && CheckValid(AllRotationsLshape[TileType][NextRotation][3] + TileCoordinate)) {
      
        // 更新旋转状态
        Rotation = NextRotation;
        for (int i = 0; i < 4; i++) 
            Tile[i] = AllRotationsLshape[TileType][Rotation][i];
        UpdateTile();  // 更新VBO
    }
    // 如果不合法则不旋转（墙壁踢回）
}
```

**技术要点**：旋转前先检测，不合法则取消旋转，实现"墙壁踢回"效果。

---

### 1.3.6 消行与计分

```cpp
// 修改单个格子颜色并更新VBO
void ChangeCellColor(glm::vec2 Coordinate, glm::vec4 Color) {
    // 更新颜色数组
    for (int i = 0; i < 6; i++)
        BoardColor[6*(BoardWidth*(int)Coordinate.y + (int)Coordinate.x) + i] = Color;
  
    // 局部更新VBO
    glm::vec4 NewColors[6] = { Color, Color, Color, Color, Color, Color };
    glBindBuffer(GL_ARRAY_BUFFER, Vbo[3]);
    int Offset = 6*sizeof(glm::vec4) * (BoardWidth*(int)Coordinate.y + (int)Coordinate.x);
    glBufferSubData(GL_ARRAY_BUFFER, Offset, sizeof(NewColors), NewColors);
}

// 检查并消除满行
void CheckFullRow(int Row) {
    // 检查该行是否全部被占据
    for (int i = 0; i < BoardWidth; i++)
        if (!BoardOccupied[i][Row]) return;
  
    // 计分与难度提升
    Score += 10;
    if (Score >= Difficulty * 60) {
        Difficulty++;
        FallingSpeed /= 2;  // 下落速度翻倍
    }
  
    // 上方所有行下移一格
    for (int k = Row + 1; k < BoardHeight; k++) {
        for (int j = 0; j < BoardWidth; j++) {
            BoardOccupied[j][k-1] = BoardOccupied[j][k];
            ChangeCellColor(glm::vec2(j, k-1), BoardColor[6*(j + k*BoardWidth)]);
        }
    }
  
    // 清空最顶行
    for (int j = 0; j < BoardWidth; j++) {
        ChangeCellColor(glm::vec2(j, BoardHeight-1), Black);
        BoardOccupied[j][BoardHeight-1] = false;
    }
  
    PrintStatus();  // 打印分数
}
```

---

### 1.3.7 时间控制与自动下落

```cpp
clock_t StartTime;  // 上次下落时间戳
int FallingSpeed = 1000;  // 下落间隔（毫秒）
bool Accelerating = false;  // 加速标志

void Idle() {
    clock_t CurrentTime = clock();
  
    // 计算时间间隔（加速时60ms，正常时FallingSpeed）
    int interval = Accelerating ? 60 : FallingSpeed;
  
    if (CurrentTime - StartTime > interval) {
        StartTime = CurrentTime;  // 更新时间戳
      
        if (!GameOver && Gaming) {
            if (!MoveTile(Down)) {  // 尝试下落
                SetTile();   // 下落失败，放置方块
                NewTile();   // 生成新方块
            }
        }
    }
}
```

**技术要点**：使用 `clock()` 实现帧率无关的时间控制，游戏速度不受显示刷新率影响。

---

### 1.3.8 键盘交互

```cpp
void KeyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mode) {
    // 全局控制
    switch (Key) {
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q:
        if (Action == GLFW_PRESS) exit(EXIT_SUCCESS);
        break;
    case GLFW_KEY_R:  // 重新开始
        if (Action == GLFW_PRESS) Restart();
        break;
    case GLFW_KEY_P:  // 暂停/继续
        if (Action == GLFW_PRESS && !GameOver) {
            Gaming = !Gaming;
            std::cout << (Gaming ? "Game Continued." : "Game Paused.") << std::endl;
        }
        break;
    }
  
    // 游戏中控制
    if (!GameOver && Gaming) {
        switch (Key) {
        case GLFW_KEY_UP:  // 旋转
            if (Action == GLFW_PRESS || Action == GLFW_REPEAT) Rotate();
            break;
        case GLFW_KEY_DOWN:  // 下移
            if (Action == GLFW_PRESS || Action == GLFW_REPEAT) {
                if (!MoveTile(Down)) { SetTile(); NewTile(); }
            }
            break;
        case GLFW_KEY_LEFT:  // 左移
            if (Action == GLFW_PRESS || Action == GLFW_REPEAT) MoveTile(Left);
            break;
        case GLFW_KEY_RIGHT:  // 右移
            if (Action == GLFW_PRESS || Action == GLFW_REPEAT) MoveTile(Right);
            break;
        case GLFW_KEY_SPACE:  // 加速下落
            if (Action == GLFW_PRESS || Action == GLFW_REPEAT) Accelerating = true;
            break;
        }
    }
}
```

---

### 1.3.9 渲染流程

```cpp
void Display() {
    glClear(GL_COLOR_BUFFER_BIT);
  
    // 设置窗口大小uniform
    glUniform1i(LocXSize, XSize);
    glUniform1i(LocYSize, YSize);
  
    // 1. 绘制棋盘格子（已放置的方块）
    glBindVertexArray(Vao[1]);
    glDrawArrays(GL_TRIANGLES, 0, PointsCnt);  // 200个格子×6顶点
  
    // 2. 绘制当前下落方块
    glBindVertexArray(Vao[2]);
    glDrawArrays(GL_TRIANGLES, 0, 24);  // 4个格子×6顶点
  
    // 3. 绘制网格线（最后绘制，覆盖在上面）
    glBindVertexArray(Vao[0]);
    glDrawArrays(GL_LINES, 0, BoardLineCnt * 2);
}
```

**技术要点**：绘制顺序影响视觉效果，网格线最后绘制确保其显示在方块之上。

---

## 1.4 操作说明

| 按键 | 功能      |
| :---: | --------- |
|  ↑  | 旋转      |
|  ↓  | 下移      |
| ← → | 左右移动  |
| Space | 加速下落  |
|   P   | 暂停/继续 |
|   R   | 重新开始  |
|  ESC  | 退出      |

## 1.5 技术要点总结

| 技术点                     | 说明                                                 |
| -------------------------- | ---------------------------------------------------- |
| **VAO/VBO管理**      | 3个VAO分别管理网格线、棋盘格、当前方块               |
| **动态缓冲更新**     | `glBufferSubData` 局部更新，高效更新方块位置和颜色 |
| **帧率无关时间控制** | `clock()` 实现固定间隔下落，不受刷新率影响         |
| **碰撞检测**         | 边界检测 + 占据检测双重验证                          |
| **旋转系统**         | 预定义4个旋转状态，墙壁踢回机制                      |
| **消行算法**         | 满行检测 + 上方行整体下移 + 颜色同步更新             |
| **难度递进**         | 分数每60分提升一级，速度翻倍                         |

---

# 二、期末大作业 - 御剑飞行虚拟场景

## 2.1 项目概述

实现类似《仙剑奇侠传》御剑飞行的3D虚拟场景，包含丰富场景元素、动画和交互。

**项目路径**：`D:\learn\vcc\4.1\`

## 2.2 实验要求完成情况

### ✅ 要求1：场景设计与层级建模（≥4层）

#### 层级建模结构体

```cpp
struct PavilionPart {
    TriMesh* mesh = nullptr;
    std::vector<std::shared_ptr<PavilionPart>> children;
    glm::vec3 local_translation = glm::vec3(0.0f);
    glm::vec3 local_rotation = glm::vec3(0.0f);
    glm::vec3 local_scale = glm::vec3(1.0f);
};
```

#### 四层层级结构（亭子）

```
pavilion_root (第1层)
└── base 台基 (第2层)
    ├── step 台阶 (第3层)
    ├── pillar_0~3 柱子 (第3层)
    └── roof 屋顶 (第3层)
        └── ornament 装饰 (第4层) 
```

#### 递归渲染代码

```cpp
void drawPavilionPart(const std::shared_ptr<PavilionPart>& part, 
                      glm::mat4 parentModel, MeshPainter* painter, 
                      Light* light, Camera* camera) {
    if (!part) return;
  
    glm::mat4 model = parentModel;
    model = glm::translate(model, part->local_translation);
    model = glm::rotate(model, glm::radians(part->local_rotation.y), glm::vec3(0,1,0));
    model = glm::scale(model, part->local_scale);
  
    if (part->mesh) {
        // 使用GLM分解矩阵
        glm::vec3 scale, translation, skew;
        glm::quat orient;
        glm::vec4 perspective;
        glm::decompose(model, scale, orient, translation, skew, perspective);
      
        part->mesh->setTranslation(translation);
        part->mesh->setRotation(glm::degrees(glm::eulerAngles(orient)));
        part->mesh->setScale(scale);
        painter->drawMesh(part->mesh, ...);
    }
  
    for (auto& child : part->children)
        drawPavilionPart(child, model, painter, light, camera);
}
```

#### 场景物体统计

| 类型 |  数量  | 说明         |
| ---- | :----: | ------------ |
| 地面 |   1   | 带纹理       |
| 亭子 |   1   | 4层层级      |
| 湖泊 |   1   | 程序化水纹   |
| 小桥 |   1   | 带纹理+栏杆  |
| 荷叶 |   8   | 水面装饰     |
| 假山 |   6   | 环境装饰     |
| 仙山 |   12   | 远景         |
| 祥云 |   25   | 天空         |
| 仙宫 |   4   | 建筑群       |
| 人物 | 17部件 | 精细建模     |
| 仙鹤 | 7部件 | 第二可控物体 |

#### 场景物品代码详解

**1. 地面生成**

```cpp
TriMesh* ground = new TriMesh();
ground->generateGround(300.0);  // 生成300x300的大型地面
ground->setTranslation(glm::vec3(0.0, -3.0, 0.0));
ground->setAmbient(glm::vec4(0.3, 0.5, 0.3, 1.0));   // 草绿色环境光
ground->setDiffuse(glm::vec4(0.4, 0.65, 0.4, 1.0));  // 漫反射
ground->setSpecular(glm::vec4(0.05, 0.1, 0.05, 1.0)); // 低镜面反射
ground->setShininess(3.0);
painter->addMesh(ground, "ground", "assets/test.png", vshader, fshader);
```

**2. 湖泊与程序化水纹**

```cpp
TriMesh* lake = new TriMesh();
lake->generateGround(25.0);  // 25x25湖面
lake->setTranslation(glm::vec3(0.0, -2.8, 30.0));
lake->setAmbient(glm::vec4(0.15, 0.35, 0.55, 1.0));  // 深蓝
lake->setDiffuse(glm::vec4(0.25, 0.5, 0.75, 1.0));
lake->setSpecular(glm::vec4(0.7, 0.8, 0.9, 1.0));    // 高反射模拟水面
lake->setShininess(90.0);  // 高光泽度
painter->addMesh(lake, "lake", "procedural:water", vshader, fshader);
```

**3. 小桥与栏杆**

```cpp
// 桥面
TriMesh* bridge = new TriMesh();
bridge->generateCube();
bridge->setTranslation(glm::vec3(0.0, -1.5, 18.0));
bridge->setScale(glm::vec3(2.0, 0.3, 8.0));  // 宽2，厚0.3，长8
bridge->setAmbient(glm::vec4(0.5, 0.35, 0.2, 1.0));  // 木色
painter->addMesh(bridge, "bridge", "assets/table.png", vshader, fshader);

// 栏杆（左右各一条）
for (int i = 0; i < 2; i++) {
    TriMesh* rail = new TriMesh();
    rail->generateCube();
    float side = (i == 0) ? -1.1f : 1.1f;  // 左右偏移
    rail->setTranslation(glm::vec3(side, -0.8, 18.0));
    rail->setScale(glm::vec3(0.1, 0.5, 7.5));
    painter->addMesh(rail, "rail_" + std::to_string(i), "", vshader, fshader);
}
```

**4. 荷叶（循环生成）**

```cpp
for (int i = 0; i < 8; i++) {
    TriMesh* lotus = new TriMesh();
    lotus->generateDisk(16, 1.2 + (i%3)*0.3);  // 圆盘，半径随机变化
    // 计算随机分布位置
    float lx = (i%4 - 1.5f) * 5.0f + sin(i*1.3)*2.0;
    float lz = 28.0f + (i/4) * 6.0f + cos(i*0.9)*2.0;
    lotus->setTranslation(glm::vec3(lx, -2.7, lz));
    lotus->setRotation(glm::vec3(-90.0, i*30.0, 0.0));  // 平躺+随机旋转
    lotus->setAmbient(glm::vec4(0.15, 0.4, 0.15, 1.0)); // 荷叶绿
    painter->addMesh(lotus, "lotus_" + std::to_string(i), "", vshader, fshader);
}
```

**5. 远景仙山（环形分布）**

```cpp
for (int i = 0; i < 12; i++) {
    TriMesh* mountain = new TriMesh();
    mountain->generateCone(24, 8.0 + (i%4)*5.0, 30.0 + (i%5)*15.0);  // 圆锥山峰
    float angle = i * 2.0 * M_PI / 12.0;  // 环形均匀分布
    float radius = 90.0 + (i%3) * 30.0;   // 距离中心90~150
    mountain->setTranslation(glm::vec3(radius*cos(angle), -3.0, radius*sin(angle)));
    mountain->setAmbient(glm::vec4(0.1, 0.2, 0.12, 1.0));  // 青绿色
    painter->addMesh(mountain, "mountain_" + std::to_string(i), "", vshader, fshader);
}
```

**6. 祥云（随机高度分布）**

```cpp
for (int i = 0; i < 25; i++) {
    TriMesh* cloud = new TriMesh();
    cloud->generateCube();
    float angle = i * 2.0 * M_PI / 25.0 + (i%3)*0.2;
    float radius = 40.0 + (i%6)*15.0;
    float height = 12.0 + (i%7)*5.0;  // 高度12~47
    cloud->setTranslation(glm::vec3(radius*cos(angle), height, radius*sin(angle)));
    cloud->setScale(glm::vec3(4.0+(i%4)*1.5, 0.7+(i%3)*0.3, 2.5+(i%3)*1.0));  // 扁平
    cloud->setAmbient(glm::vec4(0.88, 0.9, 0.95, 1.0));  // 白色
    painter->addMesh(cloud, "cloud_" + std::to_string(i), "", vshader, fshader);
}
```

**7. 仙宫建筑群（四方位）**

```cpp
for (int i = 0; i < 4; i++) {
    float angle = i * M_PI / 2.0 + M_PI/4.0;  // 45°、135°、225°、315°
    float radius = 55.0;
    float bx = radius * cos(angle), bz = radius * sin(angle);
  
    // 白玉台基
    TriMesh* platform = new TriMesh();
    platform->generateCube();
    platform->setTranslation(glm::vec3(bx, -1.0, bz));
    platform->setScale(glm::vec3(8.0, 2.0, 8.0));
    platform->setAmbient(glm::vec4(0.75, 0.75, 0.78, 1.0));  // 白玉色
  
    // 朱红殿身
    TriMesh* hall = new TriMesh();
    hall->generateCube();
    hall->setTranslation(glm::vec3(bx, 3.5, bz));
    hall->setScale(glm::vec3(6.0, 5.0, 6.0));
    hall->setAmbient(glm::vec4(0.5, 0.2, 0.15, 1.0));  // 朱红色
  
    // 青瓦飞檐（四角锥）
    TriMesh* roof = new TriMesh();
    roof->generateCone(4, 7.5, 3.5);  // 4边锥体模拟四角飞檐
    roof->setTranslation(glm::vec3(bx, 8.5, bz));
    roof->setRotation(glm::vec3(0.0, 45.0, 0.0));  // 旋转45°对齐
    roof->setAmbient(glm::vec4(0.1, 0.2, 0.25, 1.0));  // 青瓦色
  
    // 金顶
    TriMesh* tip = new TriMesh();
    tip->generateCone(8, 0.3, 1.2);
    tip->setTranslation(glm::vec3(bx, 12.2, bz));
    tip->setAmbient(glm::vec4(0.7, 0.6, 0.25, 1.0));  // 金色
}
```

**8. 飞剑组合（剑身+剑格+剑柄）**

```cpp
// 剑身
sword = new TriMesh();
sword->generateCube();
sword->setScale(glm::vec3(0.12, 0.025, 2.0));  // 细长
sword->setAmbient(glm::vec4(0.7, 0.6, 0.25, 1.0));
sword->setSpecular(glm::vec4(1.0, 0.95, 0.7, 1.0));  // 高光
sword->setShininess(120.0);  // 金属光泽

// 剑格（护手）
sword_guard = new TriMesh();
sword_guard->generateCube();
sword_guard->setScale(glm::vec3(0.4, 0.05, 0.1));

// 剑柄
sword_handle = new TriMesh();
sword_handle->generateCube();
sword_handle->setScale(glm::vec3(0.04, 0.03, 0.25));
sword_handle->setAmbient(glm::vec4(0.35, 0.2, 0.1, 1.0));  // 木色
```

**9. 剑客人物（17部件精细建模）**

人物由17个独立部件组成，分为头部、躯干、四肢、五官等部分：

```cpp
// ========== 头部 ==========
// 头
char_head = new TriMesh();
char_head->generateCube();
char_head->setScale(glm::vec3(0.11, 0.13, 0.1));  // 椭圆形头部
char_head->setAmbient(glm::vec4(0.7, 0.55, 0.45, 1.0));   // 肤色
char_head->setDiffuse(glm::vec4(0.95, 0.8, 0.7, 1.0));
char_head->setSpecular(glm::vec4(0.12, 0.1, 0.08, 1.0));
char_head->setShininess(8.0);

// 发髻（黑色，道士风格）
char_hair = new TriMesh();
char_hair->generateCube();
char_hair->setScale(glm::vec3(0.06, 0.1, 0.06));
char_hair->setAmbient(glm::vec4(0.08, 0.06, 0.05, 1.0));  // 深黑色
char_hair->setDiffuse(glm::vec4(0.15, 0.12, 0.1, 1.0));
char_hair->setSpecular(glm::vec4(0.35, 0.32, 0.3, 1.0));  // 发丝光泽
char_hair->setShininess(30.0);

// 披肩长发（飘逸效果）
char_hair_back = new TriMesh();
char_hair_back->generateCube();
char_hair_back->setScale(glm::vec3(0.12, 0.28, 0.04));  // 薄而长
char_hair_back->setAmbient(glm::vec4(0.08, 0.06, 0.05, 1.0));

// ========== 五官 ==========
// 左眼（黑色，带高光模拟眼神光）
char_left_eye = new TriMesh();
char_left_eye->generateCube();
char_left_eye->setScale(glm::vec3(0.025, 0.025, 0.015));  // 小方块
char_left_eye->setAmbient(glm::vec4(0.05, 0.03, 0.02, 1.0));  // 深黑
char_left_eye->setDiffuse(glm::vec4(0.1, 0.08, 0.05, 1.0));
char_left_eye->setSpecular(glm::vec4(0.5, 0.5, 0.5, 1.0));   // 高光模拟眼神光
char_left_eye->setShininess(60.0);

// 右眼（与左眼对称）
char_right_eye = new TriMesh();
char_right_eye->generateCube();
char_right_eye->setScale(glm::vec3(0.025, 0.025, 0.015));
// ... 材质同左眼

// 左眉毛（细长条）
char_left_eyebrow = new TriMesh();
char_left_eyebrow->generateCube();
char_left_eyebrow->setScale(glm::vec3(0.035, 0.01, 0.01));  // 扁平细条
char_left_eyebrow->setAmbient(glm::vec4(0.08, 0.05, 0.03, 1.0));

// 右眉毛
char_right_eyebrow = new TriMesh();
char_right_eyebrow->setScale(glm::vec3(0.035, 0.01, 0.01));

// 嘴巴（淡红色）
char_mouth = new TriMesh();
char_mouth->generateCube();
char_mouth->setScale(glm::vec3(0.04, 0.015, 0.012));
char_mouth->setAmbient(glm::vec4(0.55, 0.25, 0.25, 1.0));  // 淡红色
char_mouth->setDiffuse(glm::vec4(0.8, 0.45, 0.4, 1.0));

// ========== 躯干（道袍） ==========
// 脖子
char_neck = new TriMesh();
char_neck->generateCube();
char_neck->setScale(glm::vec3(0.06, 0.06, 0.06));
char_neck->setAmbient(glm::vec4(0.7, 0.55, 0.45, 1.0));  // 肤色

// 上身（白色道袍）
char_body_upper = new TriMesh();
char_body_upper->generateCube();
char_body_upper->setScale(glm::vec3(0.2, 0.25, 0.11));
char_body_upper->setAmbient(glm::vec4(0.78, 0.78, 0.82, 1.0));  // 淡白色
char_body_upper->setDiffuse(glm::vec4(0.97, 0.97, 0.99, 1.0));
char_body_upper->setSpecular(glm::vec4(0.22, 0.22, 0.25, 1.0));  // 丝绸质感

// 腰带（金色）
char_belt = new TriMesh();
char_belt->generateCube();
char_belt->setScale(glm::vec3(0.21, 0.04, 0.12));
char_belt->setAmbient(glm::vec4(0.65, 0.55, 0.22, 1.0));  // 金色
char_belt->setDiffuse(glm::vec4(0.88, 0.75, 0.35, 1.0));
char_belt->setSpecular(glm::vec4(0.75, 0.65, 0.4, 1.0));  // 金属光泽
char_belt->setShininess(45.0);

// 下身长袍
char_body_lower = new TriMesh();
char_body_lower->generateCube();
char_body_lower->setScale(glm::vec3(0.22, 0.35, 0.12));
char_body_lower->setAmbient(glm::vec4(0.75, 0.75, 0.8, 1.0));

// 飘逸长袍下摆
char_robe = new TriMesh();
char_robe->generateCube();
char_robe->setScale(glm::vec3(0.26, 0.18, 0.14));  // 略宽，飘逸感

// ========== 四肢 ==========
// 左臂袖子
char_left_arm = new TriMesh();
char_left_arm->generateCube();
char_left_arm->setScale(glm::vec3(0.06, 0.18, 0.06));
char_left_arm->setAmbient(glm::vec4(0.58, 0.68, 0.78, 1.0));  // 略带蓝色
char_left_arm->setDiffuse(glm::vec4(0.8, 0.9, 0.98, 1.0));

// 右臂袖子
char_right_arm = new TriMesh();
char_right_arm->setScale(glm::vec3(0.06, 0.18, 0.06));

// 左手（肤色）
char_left_hand = new TriMesh();
char_left_hand->generateCube();
char_left_hand->setScale(glm::vec3(0.05, 0.06, 0.05));
char_left_hand->setAmbient(glm::vec4(0.7, 0.55, 0.45, 1.0));

// 右手
char_right_hand = new TriMesh();
char_right_hand->setScale(glm::vec3(0.05, 0.06, 0.05));

// 左脚（深色布鞋）
char_left_foot = new TriMesh();
char_left_foot->generateCube();
char_left_foot->setScale(glm::vec3(0.05, 0.03, 0.1));
char_left_foot->setAmbient(glm::vec4(0.15, 0.12, 0.1, 1.0));  // 深棕色

// 右脚
char_right_foot = new TriMesh();
char_right_foot->setScale(glm::vec3(0.05, 0.03, 0.1));
```

**人物部件层级结构**：
```
剑客人物 (17部件)
├── 头部系统
│   ├── char_head (头)
│   ├── char_hair (发髻)
│   └── char_hair_back (披肩长发)
├── 五官系统
│   ├── char_left_eye / char_right_eye (双眼)
│   ├── char_left_eyebrow / char_right_eyebrow (眉毛)
│   └── char_mouth (嘴巴)
├── 躯干系统（道袍）
│   ├── char_neck (脖子)
│   ├── char_body_upper (上身道袍)
│   ├── char_belt (金腰带)
│   ├── char_body_lower (下身长袍)
│   └── char_robe (飘逸下摆)
└── 四肢系统
    ├── char_left_arm / char_right_arm (双臂袖子)
    ├── char_left_hand / char_right_hand (双手)
    └── char_left_foot / char_right_foot (双脚)
```

**五官动态跟随计算**：
```cpp
// 五官位置随人物朝向旋转
float face_z = 0.07f;  // 面朝前方的偏移
float face_x_off = face_z * sin(angle_rad);
float face_z_off = face_z * cos(angle_rad);

// 左眼位置（相对脸部中心偏左）
float eye_x = -0.035f;
float ex_world = eye_x * cos(angle_rad) + face_x_off;
float ez_world = -eye_x * sin(angle_rad) + face_z_off;
char_left_eye->setTranslation(glm::vec3(
    sword_position_x + ex_world + sway_x,  // X: 人物位置 + 旋转后偏移 + 摇摆
    base_y + 0.97,                          // Y: 眼睛高度
    sword_position_z + ez_world             // Z: 人物位置 + 旋转后偏移
));
char_left_eye->setRotation(glm::vec3(0.0, sword_rotation_y, 0.0));
```

**技术要点**：五官相对于脸部中心有固定偏移，但需要根据人物朝向（`sword_rotation_y`）进行旋转变换，确保无论人物朝哪个方向，五官始终"面朝前方"。

**10. 仙鹤（7部件层级）**

```cpp
// 身体
crane_body = new TriMesh();
crane_body->generateCube();
crane_body->setScale(glm::vec3(0.4, 0.35, 0.8));
crane_body->setAmbient(glm::vec4(0.85, 0.85, 0.88, 1.0));  // 白色

// 头部（红色丹顶）
crane_head = new TriMesh();
crane_head->generateCube();
crane_head->setScale(glm::vec3(0.12, 0.1, 0.15));
crane_head->setAmbient(glm::vec4(0.7, 0.15, 0.1, 1.0));  // 红色

// 翅膀（黑色翅尖）
crane_wing_l = new TriMesh();
crane_wing_l->generateCube();
crane_wing_l->setScale(glm::vec3(0.8, 0.05, 0.35));
crane_wing_l->setAmbient(glm::vec4(0.1, 0.1, 0.12, 1.0));  // 黑色

// 脖子、腿等类似...
```

---

### ✅ 要求2：添加纹理（至少2个物体）

```cpp
// 地面纹理
painter->addMesh(ground, "ground", "assets/test.png", vshader, fshader);

// 小桥纹理
painter->addMesh(bridge, "bridge", "assets/table.png", vshader, fshader);

// 湖泊 - 程序化水纹
painter->addMesh(lake, "lake", "procedural:water", vshader, fshader);
```

#### 程序化水纹生成

```cpp
void MeshPainter::load_texture_STBImage(const std::string &file_name, GLuint& texture) {
    if (file_name.rfind("procedural:", 0) == 0) {
        int width = 512, height = 512;
        std::vector<unsigned char> data(width * height * 3);
      
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float u = (float)x / width, v = (float)y / height;
                float wave = 0.5f + 0.5f * sinf((u*10 + v*8) * 6.28318f);
                float r = 0.05f + 0.05f * wave;
                float g = 0.25f + 0.35f * wave;
                float b = 0.45f + 0.45f * wave;
                int idx = (y * width + x) * 3;
                data[idx] = r * 255; data[idx+1] = g * 255; data[idx+2] = b * 255;
            }
        }
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        return;
    }
    // 普通纹理加载...
}
```

---

### ✅ 要求3：光照、材质、阴影

#### Phong光照模型（片段着色器）

```glsl
// fshader.glsl
void main() {
    if (isShadow == 1) {
        fColor = vec4(0.0, 0.0, 0.0, 1.0);  // 阴影
    } else {
        vec3 N = normalize(normal);
        vec3 L = normalize(light.position - position);
        vec3 V = normalize(eye_position - position);
        vec3 R = reflect(-L, N);
      
        // 环境光
        vec4 ambient = light.ambient * material.ambient;
        // 漫反射
        float diff = max(dot(N, L), 0.0);
        vec4 diffuse = diff * light.diffuse * material.diffuse;
        // 镜面反射
        float spec = pow(max(dot(V, R), 0.0), material.shininess);
        vec4 specular = spec * light.specular * material.specular;
        // 衰减
        float dist = length(light.position - position);
        float attenuation = 1.0 / (light.constant + light.linear*dist + light.quadratic*dist*dist);
      
        fColor = ambient + (diffuse + specular) * attenuation;
    }
}
```

#### 材质示例

```cpp
// 金色飞剑
sword->setAmbient(glm::vec4(0.7, 0.6, 0.25, 1.0));
sword->setDiffuse(glm::vec4(0.95, 0.82, 0.4, 1.0));
sword->setSpecular(glm::vec4(1.0, 0.95, 0.7, 1.0));
sword->setShininess(120.0);

// 水面高反射
lake->setSpecular(glm::vec4(0.7, 0.8, 0.9, 1.0));
lake->setShininess(90.0);
```

#### 阴影实现

```cpp
void display() {
    glm::mat4 shadowMatrix = light->getShadowProjectionMatrix();
  
    // 第一遍：绘制阴影
    for (auto& mesh : meshes) {
        glm::mat4 shadowModel = shadowMatrix * mesh->getModelMatrix();
        glUniform1i(shadowLocation, 1);
        glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
    }
  
    // 第二遍：正常绘制
    painter->drawMeshes(light, camera);
}
```

---

### ✅ 要求4：相机交互（视角切换）

```cpp
void updateSwordFlying() {
    // 第三人称相机跟随
    float total_angle = sword_rotation_y + camera_angle_h;
    float cam_rad = total_angle * M_PI / 180.0;
    float v_rad = camera_angle_v * M_PI / 180.0;
  
    float cam_x = sword_position_x - camera_distance * sin(cam_rad) * cos(v_rad);
    float cam_z = sword_position_z - camera_distance * cos(cam_rad) * cos(v_rad);
    float cam_y = sword_position_y + camera_distance * sin(v_rad);
  
    camera->eye = glm::vec4(cam_x, cam_y, cam_z, 1.0);
    camera->at = glm::vec4(sword_position_x, sword_position_y, sword_position_z, 1.0);
}

// 鼠标控制视角
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (mouse_left_pressed) {
        camera_angle_h -= (xpos - last_mouse_x) * 0.3f;
        camera_angle_v += (ypos - last_mouse_y) * 0.2f;
        camera_angle_v = glm::clamp(camera_angle_v, -30.0f, 60.0f);
    }
}

// 滚轮缩放
void scroll_callback(GLFWwindow* window, double xoff, double yoff) {
    camera_distance = glm::clamp(camera_distance - yoff * 0.5f, 2.0f, 15.0f);
}
```

---

### ✅ 要求5：交互控制物体（至少2个）

#### 物体1：御剑人物

```cpp
void key_callback(...) {
    float angle_rad = sword_rotation_y * M_PI / 180.0;
    switch (key) {
    case GLFW_KEY_W:  // 前进
        sword_position_x += fly_speed * sin(angle_rad);
        sword_position_z += fly_speed * cos(angle_rad);
        break;
    case GLFW_KEY_S:  // 后退
        sword_position_x -= fly_speed * sin(angle_rad);
        sword_position_z -= fly_speed * cos(angle_rad);
        break;
    case GLFW_KEY_A: sword_rotation_y += rotation_speed; break;  // 左转
    case GLFW_KEY_D: sword_rotation_y -= rotation_speed; break;  // 右转
    case GLFW_KEY_Q: sword_position_y += fly_speed * 0.6f; break; // 上升
    case GLFW_KEY_E: sword_position_y -= fly_speed * 0.6f; break; // 下降
    }
}
```

#### 物体2：仙鹤（TAB切换）

```cpp
bool control_crane = false;

// TAB键切换控制对象
case GLFW_KEY_TAB:
    control_crane = !control_crane;
    std::cout << (control_crane ? "控制: 仙鹤" : "控制: 人物") << std::endl;
    break;

// 根据控制对象选择操作目标
float* current_rotation = control_crane ? &crane_rotation : &sword_rotation_y;
float* current_x = control_crane ? &crane_x : &sword_position_x;
float* current_y = control_crane ? &crane_y : &sword_position_y;
float* current_z = control_crane ? &crane_z : &sword_position_z;
float angle_rad = (*current_rotation) * M_PI / 180.0;

// WASD控制当前选中对象
case GLFW_KEY_W:
    *current_x += actual_speed * sin(angle_rad);
    *current_z += actual_speed * cos(angle_rad);
    break;
// A/D/Q/E 同理操作 current_rotation/current_y
```

#### 相机跟随切换（关键修复）

```cpp
// 相机根据控制对象切换跟随目标
float target_x, target_y, target_z, target_rotation;
if (control_crane) {
    // 跟随仙鹤
    target_x = crane_x;
    target_y = crane_y;
    target_z = crane_z;
    target_rotation = crane_rotation;
} else {
    // 跟随御剑人物
    target_x = sword_position_x;
    target_y = sword_position_y;
    target_z = sword_position_z;
    target_rotation = sword_rotation_y;
}

// 计算相机位置
float total_cam_angle = target_rotation + camera_angle_h;
float cam_x = target_x - camera_distance * sin(cam_angle_rad) * cos(cam_v_rad);
float cam_z = target_z - camera_distance * cos(cam_angle_rad) * cos(cam_v_rad);
float cam_y = target_y + camera_distance * sin(cam_v_rad);

camera->eye = glm::vec4(cam_x, cam_y + 0.5, cam_z, 1.0);
camera->at = glm::vec4(target_x, target_y + 0.6, target_z, 1.0);
```

#### 仙鹤动画

```cpp
// 仙鹤浮动与翅膀扇动
float crane_bob = sin(time_counter * 1.2) * 0.08;
float wing_flap = sin(time_counter * 3.0) * 20.0;  // 翅膀扇动角度

crane_body->setTranslation(glm::vec3(crane_x, crane_y + crane_bob, crane_z));
crane_body->setRotation(glm::vec3(0.0, crane_rotation, 0.0));

// 翅膀扇动
crane_wing_l->setRotation(glm::vec3(wing_flap, crane_rotation, 0.0));
crane_wing_r->setRotation(glm::vec3(-wing_flap, crane_rotation, 0.0));  // 对称扇动
```

---

## 2.3 额外功能

### 🌟 精致人物（17部件）

头部、发髻、长发、脖子、上身、腰带、下身、袍摆、左右臂、左右手、左右眼、眉毛、嘴巴、左右脚

### 🌟 飞行动画

```cpp
float bob = sin(time_counter) * 0.05f;      // 浮动
float sway = sin(time_counter * 0.5) * 1.5; // 摆动
float arm_swing = sin(time_counter * 0.8) * 0.01; // 袖子摆动
```

### 🌟 剑气拖尾

```cpp
for (int i = 0; i < 5; i++) {
    float trail_off = (i + 1) * 0.4f;
    trail_meshes[i]->setTranslation(glm::vec3(
        sword_x - trail_off * sin(angle_rad),
        sword_y * (1.0 - i*0.02),
        sword_z - trail_off * cos(angle_rad)));
}
```

## 2.4 操作说明

|   按键   | 功能         |
| :------: | ------------ |
|   W/S   | 前进/后退    |
|   A/D   | 左转/右转    |
|   Q/E   | 上升/下降    |
|  Shift  | 加速         |
|   Tab   | 切换控制对象 |
|  Space  | 重置位置     |
| 鼠标拖动 | 旋转视角     |
|   滚轮   | 缩放         |
|    H    | 帮助         |
|   ESC   | 退出         |

---

# 三、核心技术总结

## 3.1 OpenGL 渲染管线

### 管线流程
```
应用阶段          几何阶段                    光栅化阶段
   │                │                           │
顶点数据 ──→ 顶点着色器 ──→ 图元装配 ──→ 光栅化 ──→ 片段着色器 ──→ 帧缓冲
   │          (MVP变换)      (三角形)    (插值)    (Phong光照)    (输出)
```

### 核心数据结构
```cpp
// VAO (Vertex Array Object) - 顶点属性配置容器
GLuint VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

// VBO (Vertex Buffer Object) - 顶点数据存储
GLuint VBO;
glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

// 顶点属性指针配置
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset);
glEnableVertexAttribArray(0);
```

### 绘制模式对比
| 模式 | 用途 | 示例 |
|------|------|------|
| `GL_TRIANGLES` | 独立三角形 | 3D模型面片 |
| `GL_LINES` | 独立线段 | 网格线 |
| `GL_TRIANGLE_FAN` | 扇形三角形 | 圆形、圆锥底面 |
| `GL_TRIANGLE_STRIP` | 条带三角形 | 圆柱侧面 |

---

## 3.2 变换矩阵系统

### MVP 矩阵详解
```cpp
// 最终顶点位置 = Projection × View × Model × 顶点
gl_Position = projection * view * model * vec4(position, 1.0);
```

#### Model 矩阵（模型变换）
```cpp
// 变换顺序：先缩放 → 再旋转 → 最后平移
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, translation);  // 3. 平移
model = glm::rotate(model, angle, axis);     // 2. 旋转
model = glm::scale(model, scale);            // 1. 缩放

// 注意：代码书写顺序与实际作用顺序相反！
// 矩阵乘法：M = T × R × S，作用于顶点：v' = T × R × S × v
//          最后写的S先作用于顶点
```

#### View 矩阵（观察变换）
```cpp
// lookAt 函数：定义相机位置、目标点、上方向
glm::mat4 view = glm::lookAt(
    cameraPos,    // 相机位置 (eye)
    targetPos,    // 观察目标 (center)
    upVector      // 上方向 (up)
);

// 本项目：第三人称跟随相机
float cam_x = target_x - camera_distance * sin(glm::radians(target_rotation));
float cam_z = target_z - camera_distance * cos(glm::radians(target_rotation));
view = glm::lookAt(
    glm::vec3(cam_x, target_y + camera_height, cam_z),  // 在人物后上方
    glm::vec3(target_x, target_y, target_z),            // 看向人物
    glm::vec3(0.0, 1.0, 0.0)                            // Y轴朝上
);
```

#### Projection 矩阵（投影变换）
```cpp
// 透视投影：近大远小
glm::mat4 projection = glm::perspective(
    glm::radians(fov),    // 视野角度 (Field of View)
    aspect,               // 宽高比
    nearPlane,            // 近裁剪面
    farPlane              // 远裁剪面
);
```

---

## 3.3 层级建模（Hierarchical Modeling）

### 原理
```
父物体变换会影响所有子物体
最终变换 = M_root × M_parent × M_child × 顶点位置
```

### 本项目实现
```cpp
// 亭阁层级结构
struct PavilionPart {
    glm::vec3 localPosition;  // 相对父级的偏移
    glm::vec3 localScale;
    glm::vec3 localRotation;
    TriMesh* mesh;
    std::vector<PavilionPart*> children;  // 子部件
};

// 递归计算世界变换
void drawPavilionPart(PavilionPart* part, glm::mat4 parentTransform) {
    // 1. 构建局部变换矩阵
    glm::mat4 localMatrix = glm::mat4(1.0f);
    localMatrix = glm::translate(localMatrix, part->localPosition);
    localMatrix = glm::rotate(localMatrix, ...);
    localMatrix = glm::scale(localMatrix, part->localScale);
    
    // 2. 世界变换 = 父变换 × 局部变换
    glm::mat4 worldMatrix = parentTransform * localMatrix;
    
    // 3. 从世界矩阵提取位置/旋转/缩放
    glm::vec3 worldPos, worldScale;
    glm::quat worldRot;
    glm::decompose(worldMatrix, worldScale, worldRot, worldPos, ...);
    
    // 4. 应用到网格
    part->mesh->setTranslation(worldPos);
    part->mesh->setRotation(glm::degrees(glm::eulerAngles(worldRot)));
    
    // 5. 递归处理子部件
    for (auto* child : part->children) {
        drawPavilionPart(child, worldMatrix);
    }
}
```

### 层级树示例（亭阁）
```
亭阁根节点 (pavilion_root)
├── 第1层：阁楼 (pavilion_level1)
│   ├── 第2层：阁楼 (pavilion_level2)
│   │   ├── 第3层：阁楼 (pavilion_level3)
│   │   │   └── 第4层：塔尖 (pavilion_top)
│   │   └── 屋顶3 (roof3)
│   └── 屋顶2 (roof2)
└── 屋顶1 (roof1)
```

---

## 3.4 Phong 光照模型

### 三分量组成
```glsl
// 片段着色器
vec3 phong = ambient + diffuse + specular;
```

#### 1. 环境光（Ambient）
```glsl
vec3 ambient = light.ambient * material.ambient;
// 模拟间接光照，确保阴影区域不会全黑
```

#### 2. 漫反射（Diffuse）
```glsl
vec3 norm = normalize(normal);
vec3 lightDir = normalize(lightPos - fragPos);
float diff = max(dot(norm, lightDir), 0.0);  // Lambert余弦定律
vec3 diffuse = light.diffuse * diff * material.diffuse;
```

#### 3. 镜面反射（Specular）
```glsl
vec3 viewDir = normalize(viewPos - fragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * spec * material.specular;
```

### 材质参数设计
```cpp
// 金属材质（剑身）- 高光泽度
sword->setAmbient(glm::vec4(0.7, 0.6, 0.25, 1.0));
sword->setSpecular(glm::vec4(1.0, 0.95, 0.7, 1.0));
sword->setShininess(120.0);  // 高shininess = 小高光点

// 布料材质（道袍）- 柔和光泽
robe->setAmbient(glm::vec4(0.78, 0.78, 0.82, 1.0));
robe->setSpecular(glm::vec4(0.22, 0.22, 0.25, 1.0));
robe->setShininess(10.0);    // 低shininess = 大高光区域

// 水面材质 - 强反射
lake->setSpecular(glm::vec4(0.7, 0.8, 0.9, 1.0));
lake->setShininess(90.0);
```

---

## 3.5 纹理映射

### 纹理加载流程
```cpp
// 1. 生成纹理对象
GLuint texture;
glGenTextures(1, &texture);
glBindTexture(GL_TEXTURE_2D, texture);

// 2. 设置纹理参数
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 3. 加载图像数据
int width, height, channels;
unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
glGenerateMipmap(GL_TEXTURE_2D);
```

### 程序化纹理生成
```cpp
// 水波纹理 - 数学函数生成
for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
        float u = (float)x / width;
        float v = (float)y / height;
        
        // 正弦波叠加产生波纹效果
        float wave = 0.5f + 0.5f * sinf((u * 10.0f + v * 8.0f) * 6.28318f);
        
        // 蓝色渐变
        data[idx + 0] = (0.05f + 0.05f * wave) * 255;  // R
        data[idx + 1] = (0.25f + 0.35f * wave) * 255;  // G
        data[idx + 2] = (0.45f + 0.45f * wave) * 255;  // B
    }
}
```

---

## 3.6 动态缓冲更新

### GL_STATIC_DRAW vs GL_DYNAMIC_DRAW
```cpp
// 静态数据（顶点位置不变）
glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

// 动态数据（颜色、位置会变化）
glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
```

### 局部更新（高效）
```cpp
// 只更新缓冲区的一部分，避免重新分配整个缓冲区
glBufferSubData(GL_ARRAY_BUFFER, offset, size, newData);

// 示例：更新方块位置
for (int i = 0; i < 4; i++) {
    glm::vec4 newPoints[6] = { p1, p2, p3, p2, p3, p4 };
    glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(glm::vec4), 6 * sizeof(glm::vec4), newPoints);
}
```

---

## 3.7 交互控制系统

### 键盘回调
```cpp
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_W: move_forward = true; break;
            case GLFW_KEY_S: move_backward = true; break;
            case GLFW_KEY_A: turn_left = true; break;
            case GLFW_KEY_D: turn_right = true; break;
            case GLFW_KEY_TAB: control_crane = !control_crane; break;  // 切换控制对象
        }
    }
    if (action == GLFW_RELEASE) {
        // 释放时停止移动
    }
}
```

### 鼠标控制（视角）
```cpp
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (mouse_pressed) {
        float dx = xpos - last_mouse_x;
        float dy = ypos - last_mouse_y;
        
        camera_yaw += dx * mouse_sensitivity;    // 水平旋转
        camera_pitch -= dy * mouse_sensitivity;  // 垂直旋转
        
        // 限制俯仰角
        camera_pitch = glm::clamp(camera_pitch, -89.0f, 89.0f);
    }
    last_mouse_x = xpos;
    last_mouse_y = ypos;
}
```

---

## 3.8 帧率无关的动画系统

### 时间控制
```cpp
// 俄罗斯方块：固定间隔下落
clock_t currentTime = clock();
if (currentTime - lastTime > FallingSpeed) {
    lastTime = currentTime;
    MoveTile(Down);
}

// 御剑飞行：基于deltaTime的平滑动画
float deltaTime = currentFrame - lastFrame;
position += velocity * deltaTime;
rotation += angular_velocity * deltaTime;
```

### 动画函数
```cpp
// 飞剑悬浮动画 - 正弦波
float hover = sin(glfwGetTime() * 2.0) * 0.15;  // 上下浮动
float sway = sin(glfwGetTime() * 1.5) * 0.08;   // 左右摇摆

// 仙鹤翅膀扇动
float wing_angle = sin(glfwGetTime() * 4.0) * 15.0;  // ±15°扇动
crane_wing_l->setRotation(glm::vec3(0, 0, wing_angle));
crane_wing_r->setRotation(glm::vec3(0, 0, -wing_angle));  // 对称

// 长发飘动
float wind = sin(glfwGetTime() * 1.8) * 5.0;
char_hair_back->setRotation(glm::vec3(wind, 0, 0));
```

---

# 四、编译运行指南

```powershell
# 期中作业
cd D:\learn\vcc\code
cmake -B build
cmake --build build --config Release
.\build\Release\main.exe

# 期末作业
cd D:\learn\vcc\4.1
cmake -B build
cmake --build build --config Release
.\build\Release\main.exe
```

**注意**：确保 `shaders/` 和 `assets/` 在可执行文件目录下

---

# 五、亮点总结

| 亮点        | 说明                           |
| ----------- | ------------------------------ |
| 程序化纹理  | 无需外部资源，代码生成水面纹理 |
| 4层层级建模 | 使用 glm::decompose 分解矩阵   |
| 双物体控制  | Tab切换控制人物/仙鹤           |
| 精致人物    | 17部件独立动画                 |
| 丰富场景    | 80+物体构成仙境                |
| 完整光照    | Phong + 衰减 + 阴影            |

---

> **报告日期**：2025年12月26日



