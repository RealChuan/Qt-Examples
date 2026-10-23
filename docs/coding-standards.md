# 编码规范

> 这是一个**给人读、给人抄的模板**：把 Qt 插件机制和 CMake/qmake → 打包发布的全链路讲清楚，比把某个功能做完整更重要。机制优先于功能，链路优先于局部优雅，可移植与两链同构优先于省事，不写只有本仓库才懂的抽象。
>
> 下面只列"看代码不一定猜得到"的约定。

## 1. 文件

- `.hpp` / `.cc`，统一 `#pragma once`；一个模块一个目录，全小写（`pluginmanager.cc`）。
- 文件名取类名小写；模块导出宏放 `<模块>_global.hpp`。

## 2. 命名

| 对象                   | 规则                                            |
| ---------------------- | ----------------------------------------------- |
| 类型 / 枚举 / 命名空间 | PascalCase，接口加 `I` 前缀（`IPage`）          |
| 函数 / 变量 / 参数     | camelCase（`loadPlugins()`）                    |
| 私有实现类             | `<类名>Private`（`HelloPagePrivate`）           |
| 常量                   | `k` 前缀（`PageOrder::kFirst`），放匿名命名空间 |
| 布尔取值               | `is` / `has` 前缀（`isEnabled()`）              |
| 槽                     | `onXxx`（`onStartClicked()`）                   |

- QObject / QWidget 类不用 `m_` 前缀，成员一律进 PIMPL；纯值类型（`FilePath`、`Result`）例外，可用 `m_` 前缀。

## 3. PIMPL 与 QObject 的分工

```cpp
class HelloPagePrivate;                 // 头文件：只有前向声明
std::unique_ptr<HelloPagePrivate> d_ptr;
HelloPage::~HelloPage() = default;      // 析构必须 out-of-line
```

- QObject / QWidget 一律 PIMPL，头文件里不出现数据成员。
- `Private` 只放数据与算法：**不继承 QObject、不发信号、不 `connect`、不 `tr()`**。
- QObject 类按序负责：`setupUi()`（建控件）→ `retranslateUi()`（全部文案）→ `setupConnections()`（全部连接）。

## 4. 信号槽

- `connect` 集中写在类内部的 `setupConnections()` 中。
- **优先具名槽**（`onXxx`，声明必须带 `slots`）；完全不需要成员的用自由函数。
- lambda 只用于**不碰成员**的场景：槽体一旦要调用成员函数或读写成员变量（含 `d_ptr`），就必须写成具名槽。
- 槽只做转发，具体逻辑交回 `d_ptr->xxx()`。

```cpp
connect(d_ptr->timer, &QTimer::timeout, this, &PomodoroPage::onTick);
connect(d_ptr->statusTimer, &QTimer::timeout, d_ptr->status, &QLabel::hide);
```

## 5. 其余约定

- getter 用 `[[nodiscard]]` + 尾置返回：`auto id() const -> QString`。
- 失败返回 `Utils::Result<>`（`std::expected`），异常不跨插件边界。
- 导出宏三态：`<MODULE>_LIBRARY` / `<MODULE>_STATIC_LIBRARY` / 其余 import。
- `new` 只用于 Qt 父子树；平台差异收在单个 `if(APPLE) ... else()`；纯定时器不开线程，重计算才 `QtConcurrent`；魔法数字改 `constexpr`。
- `tr()` 只在 QObject 类里调用，配 `changeEvent(QEvent::LanguageChange)` → `retranslateUi()`；注释写"为什么"。

## 6. 提交前

- 源文件增删要同时改 `CMakeLists.txt` 与 `.pro`；目标名 = 目录名；禁止 `file(GLOB)`。
- 跑 `scripts/windows/format.ps1`（CI 用 `-Check`）。

## 7. 单元测试

- 测试与模块同目录 `src/<模块>/<模块>_unittest.cc`，`BUILD_TESTS`（默认 ON）门控；无网络测试用 `qt_add_unit_test()` 注册进 CTest，依赖测试服务器的只编译、不进 CTest。
- 机制分两层：`cmake/UnitTests.cmake`（纯 C++ `add_unit_test`）+ `cmake/QtUnitTests.cmake`（Qt 入口 `qt_add_unit_test`）。
