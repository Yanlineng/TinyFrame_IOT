/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-17     yll       the first version
 */
#include<Package.h>
#include<TinyFrame.h>

static TinyFrame *pc_tf;
static void (*callback)(package *pkg);    //全局变量存储函数指针，回调函数

static package* unpack(TF_Msg *msg){      //解包函数
    package *unpack_buff;
    unpack_buff = malloc( (msg->len + 2) * sizeof(uint8_t));
    unpack_buff->type = msg->type;
    unpack_buff->len = msg->len;
    memcpy(&unpack_buff->common , msg->data, unpack_buff->len);
    return unpack_buff;
}

static TF_Result myListener(TinyFrame *tf, TF_Msg *msg)  //消息监听并解析
{
#if DEBUG
    rt_kprintf("OK - Listener triggered for msg!\n");
#endif
    package *unpack_buff;
    unpack_buff = malloc( (msg->len + 2) * sizeof(uint8_t));
    unpack_buff = unpack(msg);   //解包
    callback(unpack_buff);    //调用回调函数
    free(unpack_buff);
    return TF_STAY;
}

void Tf_init(){   //初始化
    pc_tf = TF_Init(TF_SLAVE);
    TF_AddGenericListener(pc_tf, myListener);   //绑定监听函数
}

void Register(void (*cb)(package *pkg)){   //注册回调函数
    callback=cb;
}

void Send(package *pkg){  //打包
    TF_Msg pack_buff;
    pack_buff.type = pkg->type;
    pack_buff.len = pkg->len;
    memcpy(pack_buff.data, &pkg->common, pkg->len);
    TF_Send(pc_tf, &pack_buff);
}

void * Tf_get()
{
    return pc_tf;
}

