/*
*******************************************************************************
* Name: Levi Terry
* Email: lterry80052@uat.edu
* Date: 5/28/25
* CSC230
* 
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core2 sample source code
*                          配套  M5Core2 示例源代码
* Visit for more information: https://docs.m5stack.com/en/core/core2
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/core/core2
*
* sDescribe: Hello World
* Date: 2021/7/21
*******************************************************************************
*/
#include <M5Core2.h>

/* After M5Core2 is started or reset
the program in the setUp () function will be run, and this part will only be run
once. 在 M5Core2
启动或者复位后，即会开始执行setup()函数中的程序，该部分只会执行一次。 */
void setup() {
    M5.begin();  // Init M5Core2.  初始化 M5Core2
    /* Power chip connected to gpio21, gpio22, I2C device
           Set battery charging voltage and current
           If used battery, please call this function in your project */
    M5.Lcd.print("Hello World");  // Print text on the screen (string)
                                  // 在屏幕上打印文本(字符串)
}

/* After the program in setup() runs, it runs the program in loop()
The loop() function is an infinite loop in which the program runs repeatedly
在setup()函数中的程序执行完后，会接着执行loop()函数中的程序
loop()函数是一个死循环，其中的程序会不断的重复运行 */
void loop() {
}