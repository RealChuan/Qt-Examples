# Qt的一些简单常用示例     
## 一、[Battery](Battery/)——电池；  

<table>
    <tr>
        <td ><div align=center><img src="Battery/picture/AlarmBattery.png"></div></td >
        <td ><div align=center><img src="Battery/picture/HealthyBattery.png"></div></td>
    </tr>
</table>

## 二、[BubbleWindow](BubbleWindow/)——气泡式对话框，也可作工具提示（ToolTip）；   

<div align=center><img src="BubbleWindow/picture/Bubble.png"></div>  

## 三、[Chart](Chart/)——可视化图表绘制，参考[ 使用 QChart 显示实时动态曲线 ](https://qtdebug.com/qtbook-paint-realtime-curve-qchart/ "qtdebug/公孙二狗") 和QChart相关示例；  

<div align=center><img src="Chart/picture/Chart_1.png">图一二是动态曲线</div>  
<div align=center><img src="Chart/picture/Chart_2.png">图一二是动态曲线， 图二坐标轴也会动态变化</div>  

## 四、[Clock](/Clock)——时钟；  

<div align=center><img src="Clock/picture/Clock.png"></div>  

## 五、[DashBoard](/DashBoard)——仪表盘；  

<div align=center><img src="DashBoard/picture/DashBoard.png"></div>

## 六、[DragDrop](DragDrop/)——简单控件拖拽，参考QT示例Drag and Drop Puzzle Example；  

## 七、[HttpClient ](/HttpClient)——http客户端；  

## 八、[GridViewModel ](/GridViewModel)——基于QListView的自适应宫图；  

<div align=center><img src="GridViewModel/picture/GridView.png"></div>  

## 九、[LogAsynchronous](LogAsynchronous/)——异步日志，开辟一个线程专门往文件里写日志，前后端分离。  

1. 日志文件名：应用程序名（appname）.时间(time,精确到秒).主机hostname.进程ID（Tid）.log（.count），假如一天内写的单个日志大约接近1G，会自动加后缀（.1,.2.3...，以此类推）新建新的日志文件去写，每天0点依然会rollFile；  
   1. 正常文件名：LogAsynchronous.2020-04-26-20-29-03.Youth.11828.log；  
   2. 当日写日志接近1G，新建的文件名：LogAsynchronous.2020-04-26-20-38-55.Youth.11828.log.1；  
2. 日志格式：时间（time，精确到毫秒）.线程ID（Pid）.日志级别（debug）.打印信息（msg）.文件（File）.行号（Line）。  
1. 比如：2020-04-26 20:38:55.818 2052 [Debug] 123456789qwertyuioplkjhgfdsa 8412789-File:(..\logAsynchronous\main.cpp) Line:(19)；  

## 十、[MulClient](MulClient/)——多线程客户端，一个线程一个客户端（怎么可以绕开系统限制，模拟百万个客户端）；  

## 十一、[MulServer](MulServer/)——多线程服务端，一个线程一个客户端处理（处理实时性很高的TCP通讯）；  

## 十二、[NavigationProgressBar](/NavigationProgressBar)——导航进度栏;  

<div align=center><img src="NavigationProgressBar/picture/NavigationProgressBar.png"></div>  

## 十三、[ProgressArc](ProgressArc/)——圆弧进度条；  

 <div align=center><img src="ProgressArc/picture/ProgressArc.png"></div>  

## 十四、[ReactorServer](ReactorServer/)——多线程服务端，Reactor模式（Echo）；   

## 十五、[SimpleUdp](SimpleUdp/)——简单UDP例子，广播和接收；

防火墙白名单。

## 十六、[SlipButton](SlipButton/)——滑动按钮；  

 另：更简单的实现：[有动画效果的 CheckBox](http://qtdebug.com/qtbook-animated-checkbox/)；  

  <table>
      <tr>
          <td ><div align=center><img src="SlipButton/picture/SlipButton_check.png"></div></td >
          <td ><div align=center><img src="SlipButton/picture/SlipButton_checked.png"></div></td>
      </tr>
  </table>  

## 十七、[SqlTabview](SqlTabview/)——SQLite数据库调用，模型方法； 

## 十八、[TabViewModel](TabViewModel/)——表格视图，DeleGate(Button and ProgressBar)（MVC）；   

 套用了旧的样式（懒），具体可以自己调整；

  <div align=center><img src="TabViewModel/picture/TabViewModelDelegate.png"></div> 

## 十九、[Thread](Thread/)——多线程例子，四种写法，控件更新在子线程；  

  <div align=center><img src="Thread/picture/Thread.png"></div>   

## 二十、[TreeViewModel](TreeViewModel/)——树形视图（MVC），QtCreator源码；  

## 二十一、[Scripts](Scripts/)——脚本

1. build_macos.py——macos Release编译脚本；
2. build_windows.py——windows Release编译脚本；

