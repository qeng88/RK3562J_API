编译库注意事项：

1.关联的头文件一定得放在HEADERS里面，不然无法进行MOC编译，导致输出库缺少信号连接

2.需要添加 CONFIG += unversioned_libname 不然会生成有符号链接的库
