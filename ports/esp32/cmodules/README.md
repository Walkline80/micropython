## 编译固件

修改在`boards`下对应开发板文件夹里的`mpconfigboard.h`文件，添加以下内容，用于注册（启用）对应的模块

```bash
#define MODULE_SMARTCONFIG_ENABLED	(1)
#define MODULE_QRCODE_ENABLED		(0)
#define MODULE_FONTLIB_ENABLED		(0)
```

> `1`：启用，`0`：禁用

再执行以下命令进行编译

```bash
cd micropython/ports/esp32
make USER_C_MODULES=../cmodules/micropython.cmake
```

> `QRCode`说明：
> 1. 模块有可能不支持`v1.19.1`及之前的版本，未测试
> 2. `qrcodegen`模块来自 [espressif/qrcode v0.1.0](https://components.espressif.com/components/espressif/qrcode)