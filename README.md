# Capture2
Traffic capture and intercept program based on Windows Filtering Platform (WFT)

基于WFP的流量捕获和拦截程序

使用Windows筛选平台，在不同的过滤层设置回调函数，实现相关功能。不同于wireshark使用NDIS驱动程序捕获数据包，本系统通过在Windows筛选平台的链路层过滤层设置回调函数，捕获原始数据包数据。同时，本系统在Windows筛选平台的传输层过滤层设置回调函数，对比拦截规则，拦截特定数据包。拦截规则包括传输层协议、出站或入站、源IP地址、目的IP地址、源端口、目的端口等。本系统分为内核态和用户态两部分，内核态程序包括流量拦截模块、与用户态通信模块、流量捕获模块，用户态程序包括与内核态通信模块、流量保存模块、拦截规则设置模块。

参考链接：



README先写到这里。有人对程序感兴趣的话，可以提issue，然后再补充READNE。






