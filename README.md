# Qt的一些简单常用示例     
1. ReactorServer-多线程服务端，Reactor模式（Echo），TcpServer类作为mainReactor（Aaccepter），每个Thread里跑一个Subreactor，建立相应的信号槽；          
2. MulServer-------多线程服务端，一个线程一个客户端处理（处理实时性很高的TCP通讯）；          
3. MulClient--------多线程客户端，一个线程一个客户端（怎么可以绕开系统限制，模拟百万个客户端）；     
4. Chart--------------可视化图表绘制，参考[ 使用 QChart 显示实时动态曲线 ](https://qtdebug.com/qtbook-paint-realtime-curve-qchart/ "qtdebug/公孙二狗") 和QChart相关示例；   
5. DragDrop--------简单控件拖拽，参考QT示例Drag and Drop Puzzle Example；   
6. Thread------------多线程例子，三种写法；       
7. SqlTabview------SQLite数据库调用，模型方法；    
8. TabView----------View/Model(MVC)模型；    

