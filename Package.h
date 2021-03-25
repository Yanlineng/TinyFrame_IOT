/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-17     yll       the first version
 */
/**
 * 此为接口头文件，数据包格式已经处理好
 */
#ifndef APPLICATIONS_PKG_CONFIG_H_
#define APPLICATIONS_PKG_CONFIG_H_

#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @brief If only need one uart, PKG_USING_UART_TWO shoule be 0.
*/
#define PKG_USING_UART_TWO  1

//包结构，依据type确定不同包的类型
typedef struct {
    uint8_t type;    //包类型
    uint8_t len;
    union{           //依据类型，存储不同的形式
        struct {     //一号包
            uint8_t cmd;
        }PK1_CMD;
        struct {     //二号包
            uint8_t data_type;
            uint8_t data[];
        }PK2_CMD_ACK;
        struct {     //三号包
            uint8_t dev_type;
            uint8_t dev_id;
            uint8_t op;
        }PK3_OP;
        struct {     //四号包
            uint8_t dev_type;
            uint8_t dev_id;
            uint8_t op;
            uint8_t res;
        }PK4_OP_ACK;
    }common;
}package;


/**
 * 功能：初始化TinyFrame，得到内部操作句柄
 * 参数：
 *      无
 * 返回值：
 *      无
*/
void Tf_init();   //TF初始化

/**
 * 功能：注册接收到包的回调函数,将函数绑定
 * 参数：
 *      (*cb) 为需要绑定的函数名
 *      (package *pkg) 为需要绑定的函数的参数
 * 返回值：
 *      无
*/
void Register(void (*cb)(package *pkg));  //注册监听回调函数

/**
 * 功能：发送数据包
 * 参数：
 *      *pkg 类型为package,组织好的数据包
 * 返回值：
 *      无
*/
void Send(package *pkg); //打包并发送数据

/**
 * 功能： 得到当前初始化成功的TinyFrame句柄
 * 参数：
 *      无
 * 返回值：
 *      未知类型指针（实际上是TinyFrame类型的指针）
*/
void * Tf_get();   //获得句柄指针


#if PKG_USING_UART_TWO

/**
 * @brief Send package to another UART, also named UART_TWO.
 * @param pkg Package to send.
 * @return None.
*/
void Send2(pakcage *pkg);

/**
 * @brief Get TinyFrame handle.
 * @return TinyFrame handle.
*/
void *Tf_get2(void);

#endif  /* PKG_USING_UART_TWO */

#ifdef __cplusplus
     }
#endif

#endif /* APPLICATIONS_PKG_CONFIG_H_ */
