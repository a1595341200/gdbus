# Gdbus
    在搭建嵌入式Linux应用软件系统框架时，常常会将其划分为好几个模块，每个模块之间的通信方式多数时候都会用到d-bus技术。目前基于d-bus技术的几种编程框架有：glib-dbus，GDbus，QtDbus，dbus-python。这5种编程框架的复杂度有高有低，最简单的应该非QtDbus莫属了，最复杂的过程要属glib-dbus。鉴于我对GDBus的熟悉程度，这里只介绍GDBus的编程过程。
    GDBus和glib-dbus都是由GNU组织开发的。GDBus可以认为是glib-dbus的升级版，其编程过程比起glib-dbus来要简单得多。网上有很多讨论glib-dbus编程的，但就是鲜有讲解GDBus编程过程的，于是便有了写下这篇文章的初衷。在展开讲解GDBus编程过程之前，希望各位看官具备下面列举的一些背景知识，以便更好的理解GDBus编程过程。不过也可以先跳过这些背景知识，在后面的讲解中如果有不理解的，可以再回过头来看。

1. d-bus介绍。这篇文章非常好。点击打开链接

2. glib-dbus和GDBus的比较。可以看这几篇文章。点击打开链接。点击打开链接

3. 使用GVariant实现数据的序列号处理。点击打开链接

好了，下面我们切入主题——GDBus编程过程详解。

## 一、 生成基于GDBus框架的接口库文件
1. 数据抽象。
        一套复杂的应用软件系统肯定会被划分成许多子模块，每个子模块只负责一个或几个有关的功能。在Linux下，我们可以将各个子模块实现为一个个进程。一套完整的系统，就需要各个进程相互配合通信，以交换数据和信息，完成应用系统要求的功能。比如，以时下流行的车载导航娱乐系统以及车载TBOX终端应用系统为例。

        车载导航娱乐系统：我们可以将其划分为这些模块——USB多媒体、蓝牙电话、蓝牙音乐、收音机、导航、手机互联、倒车后视等。这些模块都将以进程的方式实现。这些进程间就需要通信，以达到一个有机有序稳定的车载导航娱乐系统。

        车载TBOX终端应用系统：我们可以将其划分成这些模块——车辆数据采集模块、车辆数据处理模块、车辆数据存储模块、与TSP后台服务器通信模块，可能还附加与车内移动设备的通信模块。这些模块也都以进程的方式实现，他们之间的通信，我们都将采用GDBus技术。

        这么多进程需要通信，可能有一对一，可能有一对多，多对多的通信方式。为简单起见，我们假设某个系统中就只有两个进程需要通信——我们称之为Server和Client。基于GDBus的进程间通信，最最重要的就是要定义好Server和Client之间的通信接口。在GDBus编程框架下，我们使用xml文件来描述接口。按照惯常的思路，就是一种数据，一个接口，N多种数据，就要N多个接口，这样的思路简单、便于理解。比如下面的xml文件：
```
<?xml version="1.0" encoding="UTF-8" ?>  
<node name="/com/company/project/s">
  <interface name="com.company.project.s">
    <method name="SetSomeData">          <!-- method 方法：定义了server提供给client调用的接口名SetSomeData -->
        <arg type="i" name="datatype" direction="in" /> <!-- 定义了接口SetSomeData的参数1：datatype；direction表示值传递方向：“in”表示参数值的传递方向是client到server -->
	<arg type="i" name="data" direction="in" /> <!-- 同上，定义了接口函数SetSomeData的另一个参数，type表示参数类型：“i“表示为gint类型-->
    </method>
    <method name="GetSomeData"> <!-- 定义了server的另一个接口函数GetSomeData -->
        <arg type="i" name="datatype" direction="in" /> <!--  接口函数GetSomeData 的参数1，类型为gint，名字为datatype，值传递方向为 client到 server -->
        <arg type="i" name="retdata" direction="out" /> <!-- 参数2，类型为gint，名字为retdata，值传递方向为 server到client -->
    </method> 
    <signal name="SendMessage"> <!-- 此接口为 server 端使用，不提供给client。server使用此接口，主动向client端发送数据，signal定义的接口是异步的 -->
	<arg type="i" name="signal_id"> <!-- signal接口函数SendMessage 的参数-->
    </signal>
  </interface>
</node>
```
  上面的xml文件，共描述了三个接口：SetSomData、GetSomeData、SendMessage。其注释都已详细附上。

        但是，这样的接口描述方式存在一个非常不便的地方——每种数据一个接口，如果进程间需要交换的数据类型很多的时候，就需要定义非常多的接口，这样xml文件就会变得庞大；而且如果进程间交换的某些数据量很大时，这样的接口描述就无能为力了。因此，有必要找到一种通用的，又能交换大量数据的接口描述方式。

        基于上述目的，我们进一步抽象进程间的通信过程时发现——进程间通信不外乎就是下面这几种情况：
1. 进程A从进程B中获取数据

2. 进程A传递数据给进程B

3. 进程B主动传递数据给进程A
Server端提供给Client端调用的接口应该有：GET、SET；同时Server端还应具备主动向客户端发数据的能力SEND。
因此，基于上述抽象，我们的GDBus接口描述文档应该如下，保存为interface-S.xml文件：
```
<?xml version="1.0" encoding="UTF-8" ?>
<node name="/com/company/project/s">
  <interface name="com.company.project.demo.s">
    <method name="Client_request"> <!-- 提供给客户端使用的接口 -->
                <arg type="a*" name="input_array" direction="in" /> <!-- Client 传递给Server 的数据。被抽象为一个通用数组 a* -->
                <arg type="a*" name="output_array" direction="out" /> <!-- client端从server端get 的数据,通过这个参数传递到client端.也是一个抽象的数组 a*-->
                <arg type="i" name="result" direction="out" /> <!-- 接口的返回值，一个整型数 -->
    </method>
    <signal name="Server_message"> <!-- 服务端主动发送数据到客户端 -->
                <arg type="a{ii}" name="message_array" /> <!-- 由key 和 value 组成的 dictionary。仅仅是示例。可以是其他类型 -->
    </signal>
  </interface>
</node>
```

对于文档interface-S.xml中的数据类型 a*，a{ii}，大家可以自行百度 点击打开链接，或者直接看glib的源码中GVariant数据类型（gvarianttype.h）。

至此，我们已经抽象出所有的通信接口。接下来，我们要做的工作就是，将这份通用的接口文件转换为C代码文件，以便加入到我们的工程
2. 产生与xml文件对应的C源文件和库文件
我们使用GDBus提供的工具 gdbus-codegen来产生xml文件对于的C源文件。该工具在我们的Ubuntu系统中是默认安装了的。如果没有的话，需要自行下载glib库编译安装。

在终端执行如下命令：

gdbus-codegen --generate-c-code=libgdbusdemo_s interface-S.xml

或者写一个shell脚本，将上面的过程写进去，这样就不用每次敲代码，只需执行下脚本，即可产生我们需要的C源文件：libgdbusdemo_s.c libgdbusdemo_s.h。这两个源文件就是GDBus对上面的xml文件的翻译。两个源文件的内容，这里不粘贴出来，大家可以自己试试，然后看看这两个文件里面到底有什么神秘的东东。

        至此，我们就可以将这两个文件加入到我们的工程中，一起和我们的其他源文件参与编译了。

        但是，这还不是最合理的设计，我们的设计应该遵循模块化设计思想，所以，我们可以将这两个文件单独做成一个静态库。在编译我们的工程项目时，链接这个库即可。Linux下，从源文件生成静态库的方法，大家应该不陌生吧。按照下面的步骤即可实现：

1. gcc -c libgdbusdemo_s.c

2. ar crv libgdbusdemo_s.a libgdbusdemo_s.o

这样就生成了接口库文件 libgdbusdemo_s.a。该库文件server端和client都要用到，只是server端和client端所调用的接口不一样而已。

其实，这第2步的整个过程，都可以写入shell脚本文件中，然后保存为make.sh，简化了后续的开发：