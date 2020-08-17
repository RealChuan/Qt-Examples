# Qt的一些简单常用示例     
1. [Battery](Battery/)-----------------电池；  
<table>
    <tr>
        <td ><center><img src="Battery/picture/AlarmBattery.png"></center><center>Alarm Battery</center></td >
        <td ><center><img src="Battery/picture/HealthyBattery.png"></center><center>Healthy Battery</center></td>
    </tr>
</table>
2. [BubbleWindow](BubbleWindow/)-----------------气泡式对话框，也可作工具提示（ToolTip）；  
<table>
    <tr>
        <td ><center><img src="BubbleWindow/picture/Bubble.png"></center><center>Bubble Window</center></td >
        <td ><center><img src="BubbleWindow/picture/Bubble_tip.png"></center><center>Bubble Tip</center></td>
    </tr>
</table>

2. [Chart](Chart/)-----------------可视化图表绘制，参考[ 使用 QChart 显示实时动态曲线 ](https://qtdebug.com/qtbook-paint-realtime-curve-qchart/ "qtdebug/公孙二狗") 和QChart相关示例；  

   <center><img src="Chart/picture/Chart_1.png">图一二是动态曲线</center>  
   <center><img src="Chart/picture/Chart_2.png">图一二是动态曲线</center>  

3. [Clock](/Clock)-----------------时钟；  

    <center><img src="Clock/picture/Clock.png"></center>  

4. [DashBoard](/DashBoard)-----------------仪表盘；  

    <center><img src="DashBoard/picture/DashBoard.png"></center>

5. [DragDrop](DragDrop/)-----------------简单控件拖拽，参考QT示例Drag and Drop Puzzle Example；  

6. [LogAsynchronous](LogAsynchronous/)-----------------异步日志，开辟一个线程专门往文件里写日志，前后端分离。  

   1. 日志文件名：应用程序名（appname）.时间(time,精确到秒).主机hostname.进程ID（Tid）.log（.count），假如一天内写的单个日志大约接近1G，会自动加后缀（.1,.2.3...，以此类推）新建新的日志文件去写，每天0点依然会rollFile；  
      1. 正常文件名：LogAsynchronous.2020-04-26-20-29-03.Youth.11828.log；  
      2. 当日写日志接近1G，新建的文件名：LogAsynchronous.2020-04-26-20-38-55.Youth.11828.log.1；  
   2. 日志格式：时间（time，精确到毫秒）.线程ID（Pid）.日志级别（debug）.打印信息（msg）.文件（File）.行号（Line）。  
   1. 比如：2020-04-26 20:38:55.818 2052 [Debug] 123456789qwertyuioplkjhgfdsa 8412789-File:(..\logAsynchronous\main.cpp) Line:(19)；  

7. [MulClient](MulClient/)-----------------多线程客户端，一个线程一个客户端（怎么可以绕开系统限制，模拟百万个客户端）；  

8. [MulServer](MulServer/)-----------------多线程服务端，一个线程一个客户端处理（处理实时性很高的TCP通讯）；  

9. [NavigationProgressBar](/NavigationProgressBar)-----------------导航进度栏;  

    <center><img src="NavigationProgressBar/picture/NavigationProgressBar.png"></center>  

10. [ProgressArc](ProgressArc/)-----------------圆弧进度条；  

    <center><img src="ProgressArc/picture/ProgressArc.png"></center>  

11. [ReactorServer](ReactorServer/)-----------------多线程服务端，Reactor模式（Echo），TcpServer类作为mainReactor（Accepter），每个Thread里跑一个Subreactor，建立相应的信号槽，负载均衡使用轮询法（Round-Robin），由于Qt的事件循环机制和信号槽机制，可省去很多回调函数的写法（其实就是使用信号槽替代）；  

12. [SqlTabview](SqlTabview/)-----------------SQLite数据库调用，模型方法；  

13. [SlipButton](SlipButton/)-----------------滑动按钮；
<table>
    <tr>
        <td ><center><img src="SlipButton/picture/SlipButton_check.png"></center><center>滑动前</center></td >
        <td ><center><img src="SlipButton/picture/SlipButton_checked.png"></center><center>滑动后</center></td>
    </tr>
</table>  
14. [TabViewModel](TabViewModel/)-----------------表格视图（MVC）；  
15. [Thread](Thread/)-----------------多线程例子，三种写法，不包括QtConcurrent::run()；  
16. [TreeViewModel](TreeViewModel/)-----------------树形视图（MVC），QtCreator源码；  

