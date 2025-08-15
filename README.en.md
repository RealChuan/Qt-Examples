# Some simple and common examples of Qt

-   [Simplified Chinese](README.md)
-   [English](README.en.md)

## [Practical QT tips (update when you think of it) | Free will (realchuan.github.io)](https://realchuan.github.io/2021/10/12/QT%E5%AE%9E%E7%94%A8%E5%B0%8F%E6%8A%80%E5%B7%A7%EF%BC%88%E6%83%B3%E5%88%B0%E5%B0%B1%E6%9B%B4%E6%96%B0%EF%BC%89/)

## [Battery](Battery/)——Battery control

<div align="center">
  <img src="Battery/picture/AlarmBattery.png" width="40%" height="40%">
  <img src="Battery/picture/HealthyBattery.png" width="40%" height="40%">
</div>

## [BatteryQuick](BatteryQuick/)--Battery control (`QtQuick`）

## [Bootstarp](Bootstarp/)--Program startup settings and detection

1.  The read and write registry under Windows realizes self-start on the computer, and there are two locations to write;

    ```powershell
    HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run //对于所有用户
    HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run // 对于当前用户
    ```

2.  Copy the plist file to ~/Library/LaunchAgents/ under MacOS, and use the launchctl load/unload command to achieve self-start on the computer;

3.  There are two ways to do it under Ubuntu:
    1.  Use the systemctl --user enable/disable command to achieve self-start on the computer;
        1.  Copy the .service and .timer files to ~/.config/systemd/user/, and use the systemctl --user enable/disable command to achieve self-start on the computer;
        2.  The systemctl command is used for .timer files and .timer files are used for timed execution of .service files to prevent qxcbconnection: could not connect to display error after the graphical interface is started;
    2.  Copy the .desktop file under /usr/share/Application/ to ~/.config/autostart/ to realize the power-on self-start (not verified);

## [BubbleWindow](BubbleWindow/)——Bubble dialog box, which can also be used as a tooltip (ToolTip)

<div align="center">
  <img src="BubbleWindow/picture/Bubble.png" width="50%" height="50%">
</div>

## [Chart](Chart/)——Visual chart drawing, reference[Use QChart to display real-time dynamic curves](https://qtdebug.com/qtbook-paint-realtime-curve-qchart/ "qtdebug/公孙二狗")Examples related to QChart

<div align="center">
  <img src="Chart/picture/Chart_1.png" width="90%" height="90%">
  <div style="text-align: center;">图一、二动态曲线</div>
  <img src="Chart/picture/Chart_2.png" width="90%" height="90%">
  <div style="text-align: center;">图二坐标轴也会滚动</div>
</div>

## [CheckBoxStandardItem](CheckBoxStandardItem/)——The StandardItem that can be checked, and the parent node status or child node status can be automatically updated according to the checked state

<div align="center">
  <img src="CheckBoxStandardItem/picture/checkBoxStandardItem.png" width="50%" height="50%">
</div>

## [Clock](Clock/)--clock

<div align="center">
  <img src="Clock/picture/Clock.png" width="50%" height="50%">
</div>

## [DashBoard](DashBoard/)——Dashboard

<div align="center">
  <img src="DashBoard/picture/DashBoard.png" width="50%" height="50%">
</div>

## [DragDrop](DragDrop/)——Simple control drag and drop, refer to QT example Drag and Drop Puzzle Example

## [FlowLayout](FlowLayout/)——Flow Layout, from QT example Flow Layout Example

<div align="center">
  <img src="FlowLayout/picture/FlowLayout.png" width="50%" height="50%">
</div>

## [GridViewModel](GridViewModel/)——Adaptive palace diagram based on QListView

<div align="center">
  <img src="GridViewModel/picture/GridView.png" width="90%" height="90%">
</div>

## [HttpClient](HttpClient/)——Simple HTTP client, supports JSON request and upload, download, delete files and other functions

## [IconButton](IconButton/)——EventFilter and Button that support Icon following state switching

## [ImageCarousel](ImageCarousel/)——Simple picture carousel component

<div align="center">
  <img src="ImageCarousel/picture/ImageCarousel.jpg" width="90%" height="90%">
</div>

## [LoadingIndicator](LoadingIndicator/)——Loading indicator, supporting gif animation

<div align="center">
  <img src="LoadingIndicator/picture/LoadingIndicator.jpg" width="35%" height="35%">
</div>

## [LogAsynchronous](LogAsynchronous/)——Async log, create a thread to write logs specifically into the file, separate the front and back ends

1.  Log file name: application name (appname).time (time, accurate to seconds).host hostname.process ID (Tid).log (.count). If a single log written in one day is about 1G, it will automatically add the suffix (.1,.2.3..., and so on) to create a new log file to write. It will still rollFile at 0 o'clock every day;
    1.  Normal file name: LogAsynchronous.2020-04-26-20-29-03.Youth.11828.log;
    2.  The log written on that day was close to 1G, and the newly created file name is: LogAsynchronous.2020-04-26-20-38-55.Youth.11828.log.1;
2.  Log format: time (time, accurate to milliseconds). Thread ID (Pid). Log level (debug). Print information (msg). File (File). Line number (Line).
    1.  For example: 2020-04-26 20:38:55.818 2052[Debug]123456789qwertyuioplkjhgfdsa 8412789-File:(..\\logAsynchronous\\main.cpp) Line:(19)；
3.  [Qt-App](https://github.com/RealChuan/Qt-App/blob/main/src/utils/logasync.h), There is also a log encapsulation in this project, which is roughly the same as the code of this project. Since the update frequency of the two projects may be different, it is recommended to check the log module at the same time when viewing the log module.[Qt-App](https://github.com/RealChuan/Qt-App/blob/main/src/utils/logasync.h)Latest updates.

## [MulClient](MulClient/)——Multi-threaded client, one thread and one client (how can you bypass system restrictions and simulate millions of clients)

## [MulServer](MulServer/)——Multi-threaded server, one thread and one client processing (processing TCP communication with high real-time performance)

## [NavigationProgressBar](NavigationProgressBar/)——Navigation progress bar

<div align="center">
  <img src="NavigationProgressBar/picture/NavigationProgressBar.png" width="90%" height="90%">
</div>

## [PasswordLineEdit](PasswordLineEdit/)——Password input box

<div align="center">
  <img src="PasswordLineEdit/picture/HiddenPassword.png" width="40%" height="40%">
  <img src="PasswordLineEdit/picture/ShowPassword.png" width="40%" height="40%">
</div>

## [ProgressArc](ProgressArc/)——Arc progress bar

<div align="center">
  <img src="ProgressArc/picture/ProgressArc.png" width="90%" height="90%">
</div>

## [ProgressBar](ProgressBar/)——QProgressBar rounded corner alternative

<div align="center">
  <img src="ProgressBar/picture/ProgressBar.png" width="90%" height="90%">
</div>

## [ReactorServer](ReactorServer/)——Multi-threaded server, Reactor mode (Echo)

## [SimpleUdp](SimpleUdp/)——Simple UDP examples, broadcasting and receiving

## [ShowInMyComputer](ShowInMyComputer/)——Show the current application in my computer

Firewall whitelist.

## [SlipButton](SlipButton/)——Sliding button

> Also: Simpler implementation:[CheckBox with animation effect](http://qtdebug.com/qtbook-animated-checkbox/)；

<div align="center">
  <img src="SlipButton/picture/SlipButton_check.png" width="40%" height="40%">
  <img src="SlipButton/picture/SlipButton_checked.png" width="40%" height="40%">
</div>

## [SqliteWAL](SqliteWAL/)——Multi-threaded concurrent writing database program in Sqlite WAL mode

[Write-Ahead Logging (WAL) Official Documentation](https://sqlite.org/wal.html)

1.  Each thread has an independent database connection (different connection names). Thread exits requires actively removing the database connection, otherwise a large number of database connections will be generated;
2.  Best engineering practice, not best performance:
    1.  Write operation use`QMutex`, serialize the write operation to avoid[SQLITE_BUSY](https://sqlite.org/rescode.html#busy)mistake;
    2.  Reading operations do not require locking;

## [TableViewModel](TableViewModel/)——Table View

1.  Various custom agents
    1.  [ButtonDelegate](./TableViewModel/buttondelegate.h)；
    2.  [ComboBoxDelegate](./TableViewModel/comboboxdelegate.h)；
    3.  [ProgressBarDelegate](./TableViewModel/progressbardelegate.h)；
    4.  [Richtextitemdemelegate](./TableViewModel/richtextitemdelegate.hpp)；
    5.  [StarDelegate](./TableViewModel/stardelegate.h)---From Qt example Star Delegate Example;
2.  Rendering of data at a level of 100,000;

<div align="center">
  <img src="TableViewModel/picture/TabViewModelDelegate.jpg" width="90%" height="90%">
</div>

## [Thread](Thread/)——Multi-threaded examples, 6 ways to write

## [TreeViewModel](TreeViewModel/)——Tree view (MVC), QtCreator source code

<div align="center">
  <img src="TreeViewModel/picture/TreeView.png" width="90%" height="90%">
  <img src="TreeViewModel/picture/ListView.png" width="90%" height="90%">
</div>

## [Validator](Validator/)——Enhanced versions of IntValidator (QIntValidator) and DoubleValidator (QDoubleValidator)

## [packaging](packaging/)——Packaging scripts

1.  [macos](packaging/macos/)
    1.  [`qmake`](packaging/macos/build.py)Compile;
    2.  Package pkg and dmg packages and sign (`python`/`appdmg`), please refer to[Qt-App](https://github.com/RealChuan/Qt-App/tree/main/packaging/macos)；
2.  [ubuntu](packaging/ubuntu/)
    1.  [`qmake`](packaging/ubuntu/build.py)Compile;
    2.  You can refer to the package deb package[Qt-App](https://github.com/RealChuan/Qt-App/tree/main/packaging/ubuntu)；
3.  [windows](packaging/windows/)
    1.  [`qmake`](packaging/windows/build.py)Compile;
    2.  [`signtool`](packaging/windows/sign.bat)sign;
    3.  Inno Setup package can be used as a reference[Qt-App](https://github.com/RealChuan/Qt-App/tree/main/packaging/windows), if you sign it, you can[sign.bat](packaging/windows/sign.bat)Copy the signature script in the Inno Setup tool`Tools`->`Configure Sign Tools`, and then add sign flags after the required file;
