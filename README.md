# Qt的一些简单常用示例

- [简体中文](README.md)
- [English](README.en.md)

## [QT实用小技巧（想到就更新） | 自由意志 (realchuan.github.io)](https://realchuan.github.io/2021/10/12/QT%E5%AE%9E%E7%94%A8%E5%B0%8F%E6%8A%80%E5%B7%A7%EF%BC%88%E6%83%B3%E5%88%B0%E5%B0%B1%E6%9B%B4%E6%96%B0%EF%BC%89/)

## [Battery](/Battery/)——电池控件

<table>
    <tr>
        <td ><div align=center><img src="Battery/picture/AlarmBattery.png"></div></td >
        <td ><div align=center><img src="Battery/picture/HealthyBattery.png"></div></td>
    </tr>
</table>

## [BatteryQuick](/BatteryQuick/)--电池控件（`QtQuick`）

## [Bootstarp](/Bootstarp/)--程序开机自启动设置和检测

1. Windows下读写注册表实现开机自启动，有两个位置可以写入;

    ```powershell
    HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run //对于所有用户
    HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run // 对于当前用户
    ```

2. MacOS下拷贝plist文件到~/Library/LaunchAgents/，使用launchctl load/unload命令实现开机自启动;
3. Ubuntu下有两种方式：
   1. 使用systemctl --user enable/disable命令实现开机自启动;
      1. 拷贝.service和.timer文件到~/.config/systemd/user/，使用systemctl --user enable/disable命令实现开机自启动;
      2. systemctl命令用于.timer文件,.timer文件用于定时执行.service文件，防止图形界面启动后，出现qxcbconnection: could not connect to display错误;
   2. 把/usr/share/Application/下的.desktop文件拷贝到~/.config/autostart/下，实现开机自启动（未验证）;

## [BubbleWindow](/BubbleWindow/)——气泡式对话框，也可作工具提示（ToolTip）

<div align=center><img src="BubbleWindow/picture/Bubble.png"></div>

## [Chart](Chart/)——可视化图表绘制，参考[使用 QChart 显示实时动态曲线](https://qtdebug.com/qtbook-paint-realtime-curve-qchart/ "qtdebug/公孙二狗") 和QChart相关示例

<div align=center><img src="Chart/picture/Chart_1.png">图一二是动态曲线</div>  
<div align=center><img src="Chart/picture/Chart_2.png">图一二是动态曲线， 图二坐标轴也会动态变化</div>

## [CheckBoxStandardItem](/CheckBoxStandardItem/)——可以勾选的StandardItem，而且根据勾选状态自动更新父节点状态或者子节点状态

<div align=center><img src="CheckBoxStandardItem/picture/checkBoxStandardItem.png"></div>

## [Clock](/Clock/)——时钟

<div align=center><img src="Clock/picture/Clock.png"></div>

## [DashBoard](/DashBoard/)——仪表盘

<div align=center><img src="DashBoard/picture/DashBoard.png"></div>

## [FlowLayout](/FlowLayout/)——流式布局，来自QT示例Flow Layout Example

<div align=center><img src="FlowLayout/picture/FlowLayout.png"></div>

## [DragDrop](/DragDrop/)——简单控件拖拽，参考QT示例Drag and Drop Puzzle Example

## [HttpClient](/HttpClient/)——http客户端

## [IconButton](/IconButton/)——支持Icon跟随状态切换的EventFilter和Button

## [ImageCarousel](/ImageCarousel/)——简易图片轮播

<div align=center><img src="ImageCarousel/picture/ImageCarousel.jpg"></div>

## [GridViewModel](/GridViewModel/)——基于QListView的自适应宫图

<div align=center><img src="GridViewModel/picture/GridView.png"></div>

## [LogAsynchronous](/LogAsynchronous/)——异步日志，开辟一个线程专门往文件里写日志，前后端分离

1. 日志文件名：应用程序名（appname）.时间(time,精确到秒).主机hostname.进程ID（Tid）.log（.count），假如一天内写的单个日志大约接近1G，会自动加后缀（.1,.2.3...，以此类推）新建新的日志文件去写，每天0点依然会rollFile；
   1. 正常文件名：LogAsynchronous.2020-04-26-20-29-03.Youth.11828.log；
   2. 当日写日志接近1G，新建的文件名：LogAsynchronous.2020-04-26-20-38-55.Youth.11828.log.1；
2. 日志格式：时间（time，精确到毫秒）.线程ID（Pid）.日志级别（debug）.打印信息（msg）.文件（File）.行号（Line）。
   1. 比如：2020-04-26 20:38:55.818 2052 [Debug] 123456789qwertyuioplkjhgfdsa 8412789-File:(..\logAsynchronous\main.cpp) Line:(19)；
3. [Qt-App](https://github.com/RealChuan/Qt-App/blob/main/src/utils/logasync.h)，这个项目中也有对日志的封装，与本项目的代码大致一致，由于两个项目更新频率可能不同，建议在查看日志模块时，同时检查[Qt-App](https://github.com/RealChuan/Qt-App/blob/main/src/utils/logasync.h)的最新更新。

## [MulClient](/MulClient/)——多线程客户端，一个线程一个客户端（怎么可以绕开系统限制，模拟百万个客户端）

## [MulServer](/MulServer/)——多线程服务端，一个线程一个客户端处理（处理实时性很高的TCP通讯）

## [NavigationProgressBar](/NavigationProgressBar/)——导航进度栏

<div align=center><img src="NavigationProgressBar/picture/NavigationProgressBar.png"></div>

## [PasswordLineEdit](/PasswordLineEdit/)——密码输入框

<table>
    <tr>
        <td ><div align=center><img src="PasswordLineEdit/picture/HiddenPassword.png"></div></td >
        <td ><div align=center><img src="PasswordLineEdit/picture/ShowPassword.png"></div></td>
    </tr>
</table>

## [ProgressArc](/ProgressArc/)——圆弧进度条

<div align=center><img src="ProgressArc/picture/ProgressArc.png"></div>

## [ProgressBar](/ProgressBar/)——QProgressBar圆角替代方案

<div align=center><img src="ProgressBar/picture/ProgressBar.png"></div>

## [ReactorServer](/ReactorServer/)——多线程服务端，Reactor模式（Echo）

## [SimpleUdp](/SimpleUdp/)——简单UDP例子，广播和接收

## [ShowInMyComputer](/ShowInMyComputer/)——在我的电脑中显示当前应用程序

防火墙白名单。

## [SlipButton](/SlipButton/)——滑动按钮

 另：更简单的实现：[有动画效果的 CheckBox](http://qtdebug.com/qtbook-animated-checkbox/)；

<table>
      <tr>
          <td ><div align=center><img src="SlipButton/picture/SlipButton_check.png"></div></td >
          <td ><div align=center><img src="SlipButton/picture/SlipButton_checked.png"></div></td>
      </tr>
  </table>

## [SqliteWAL](/SqliteWAL/)——Sqlite WAL 模式下多线程并发写入数据库程序

### WAL模式的优点

1. 提高了并发性：WAL模式允许多个读取器和一个写入器同时访问数据库，可以提高并发性能和性能；
2. 崩溃恢复：WAL模式在发生崩溃时确保数据库保持一致，通过在提交事务之前将所有更改刷新到日志文件来实现；
3. 改进了写入性能：WAL模式允许并发写入，可以比默认的回滚模式更好地改进写入性能；

### WAL模式的注意事项

1. WAL模式仅适用于SQLite 3.35.5+版本；
2. 增加了磁盘使用量：与回滚模式相比，WAL模式需要更多的磁盘空间，因为它在提交更改之前将所有更改都写入日志文件；
3. 读取性能较慢：在WAL模式下，读取操作不会被写入操作阻塞，如果同时进行读取和写入操作，可能导致数据不一致。

## [TableViewModel](/TableViewModel/)——表格视图

1. 各种自定义代理
   1. [ButtonDelegate](./TableViewModel/buttondelegate.h)；
   2. [ComboBoxDelegate](./TableViewModel/comboboxdelegate.h)；
   3. [ProgressBarDelegate](./TableViewModel/progressbardelegate.h)；
   4. [RichTextItemDelegate](./TableViewModel/richtextitemdelegate.hpp)；
   5. [StarDelegate](./TableViewModel/stardelegate.h)----来自Qt示例Star Delegate Example；
2. 十万级数据渲染；

<div align=center><img src="TableViewModel/picture/TabViewModelDelegate.jpg"></div>

## [Thread](/Thread/)——多线程例子，6种写法

## [TreeViewModel](/TreeViewModel/)——树形视图（MVC），QtCreator源码

<div align=center><img src="TreeViewModel/picture/TreeView.png"></div>  
<div align=center><img src="TreeViewModel/picture/ListView.png"></div>

## [Validator](/Validator/)——加强版IntValidator（QIntValidator）和DoubleValidator（QDoubleValidator）

## [packaging](/packaging/)——打包脚本

1. [macos](/packaging/macos/)——macos qmake编译、打包dmg包脚本（`python`/`appdmg`）；
2. [ubuntu](/packaging/ubuntu/)——ubuntu qmake编译、打包AppImage/deb包脚本（`linuxdeployqt-continuous-x86_64.AppImage`/`dpkg-deb`）；
    1. [使用root权限打开应用程序的一种方法](/packaging/ubuntu/opt/MyApp/MyApp.sh)：

       ```shell
       #!/bin/sh
       pkexec env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY /opt/MyApp/MyApp
       ```

3. [windows](/packaging/windows/)——windows qmake编译、打包安装脚本（`Innosetup`）；
   1. `Innosetup` `signtool`

      ```
      sha256="C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\\x86\\signtool.exe" sign /f C:\\certificate\\certificate.pfx /p password /fd SHA256 /tr http://timestamp.digicert.com/scripts/timestamp.dll /td SHA256 $f
      ```
