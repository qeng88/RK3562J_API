# ============================================================
# 项目配置文件：Alson_api.pro
# 功能：构建 Alson_api 动态库
# 作者：qeng
# 日期：2026-06-16
# ============================================================

QT -= gui
QT += core serialbus

TEMPLATE = lib                  #
TARGET = Alson_api              # 动态库名称

CONFIG += c++17
CONFIG += shared                # 编译生成动态库
#CONFIG += create_prl           # 动态库依赖.prl文件
CONFIG += unversioned_libname   # 不生成符号链接

DEFINES += ALSON_API_LIBRARY    # 库导出宏，用于头文件dllexport/dllimport区分
DEFINES += QT_DEPRECATED_WARNINGS   # 启用Qt废弃接口警告，便于代码迭代整改

DESTDIR = $$PWD/lib

INCLUDEPATH += $$PWD/include            # 公共头文件搜索路径（对外给上层应用调用）
INCLUDEPATH += $$PWD/include_private    # 私有内部头文件，仅库内部使用，不对外暴露


SOURCES += \
    src/Can/canthread.cpp \
    src/Can/canworker.cpp \
    src/DevicesInfo/devicesinfo.cpp \
    src/alson_api.cpp


HEADERS += \
    include/alson_api.h \
    include_private/alson_api_global.h \
    include_private/canthread.h \
    include_private/canworker.h \
    include_private/devicesinfo.h

# 私有依赖库链接（不传递给依赖本库的上层应用）
LIBS_PRIVATE += -Wl,-Bstatic -L$$PWD/lib -lsocketcan -Wl,-Bdynamic

# 如果没有静态库(.a)，只有动态库(.so)，就用下面的方式
# LIBS_PRIVATE += -L$$PWD/lib -lsocketcan

# 确保构建 libAlson_bsp.so 时就发现未解析符号
QMAKE_LFLAGS += -Wl,-z,defs

# 运行时让 libAlson_bsp.so 从自身所在目录找 libsocketcan.so
# 这行具体转义建议用 readelf 检查最终结果
QMAKE_LFLAGS += -Wl,-rpath,\'\$$ORIGIN\'

# 设置共享库的 SONAME（用于运行时依赖识别）
QMAKE_LFLAGS_SONAME =
QMAKE_LFLAGS_SHLIB += -Wl,-soname,libAlson_api.so

# ===== 隐藏依赖 =====
CONFIG += hide_symbols
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,--exclude-libs,ALL
