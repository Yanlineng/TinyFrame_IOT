/**
 * This is an example of integrating TinyFrame into the application.
 *
 * TF_WriteImpl() is required, the mutex functions are weak and can
 * be removed if not used. They are called from all TF_Send/Respond functions.
 *
 * Also remember to periodically call TF_Tick() if you wish to use the
 * listener timeout feature.
 *
 * This is a porting configuration file, it needs to specify how to send data( in TF_WriteImpl() )
 * and add a data receiving interface( TF_AcceptChar() ) in the corresponding place
 */

/**
 *  此移植文件中，需要处理2个地方
 *
 *  1. TinyFrame发送帧的接口需要绑定，具体在 TF_WriteImpl()中设计你实际发送的方式，这里是用串口2发送数据。
 *
 *  2. TinyFrame接收帧字符接口需要在合适的地方调用，使用  TF_AcceptChar(Tf_get(), ch) 接收收到的字符数据，
 *     这里我从串口接收到字符后，继续使用 TF_AcceptChar 接收此字符至TinyFrame处理层。
 *
 **/
#include <src/TinyFrame.h>
#include <Package.h>
#include <rtdevice.h>
#include <rtthread.h>
#include "drv_common.h"

#define THREAD_PRIORITY    5
#define THREAD_STACK_SIZE  2048
#define THREAD_TIMESLICE   20

//维护一个串口接收信息的线程，并在串口接收到消息后传入 TF_AcceptChar();
static char ch;
 /* 初始化配置参数 ，用于配置波特率等*/
struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

static rt_thread_t uart2_thread = RT_NULL;

static rt_err_t uart2_input(rt_device_t dev, rt_size_t size);  //接收信息回调函数

static rt_device_t serialuart2;

static struct rt_semaphore rx_sem1;

static void USART2_SEND(const uint8_t *buff,uint32_t len)
{
    rt_device_write(serialuart2 , 0, buff, len);
    rt_kprintf("uart send!!!\n");
}

//用串口发送数据帧！,此处实现发送的具体方式
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    //TODO use UART to send message.
    USART2_SEND(buff,len);   // send to UART
}

static void uart2_thread_entry(void* parameter)
{
    while (1)
    {
        /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
        while (rt_device_read(serialuart2, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&rx_sem1, RT_WAITING_FOREVER);
        }
        TF_AcceptChar(Tf_get(), ch);  //TF帧按照字节收取数据
    }
}
//回调函数
static rt_err_t uart2_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem1);
    return RT_EOK;
}

//串口进程初始化 ---- rt-thread
static int usart_rcv_start()
{
    serialuart2 = rt_device_find("uart2");

    /* step：修改串口配置参数 */
    config.baud_rate = BAUD_RATE_115200;      //修改波特率为 115200
    config.data_bits = DATA_BITS_8;           //数据位 8
    config.stop_bits = STOP_BITS_1;           //停止位 1
    config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
    config.parity    = PARITY_NONE;           //无奇偶校验位

    /* step：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(serialuart2, RT_DEVICE_CTRL_CONFIG, &config);

    /* 以 DMA 接收及轮询发送方式打开串口设备 */
    rt_err_t res = rt_device_open(serialuart2,  RT_DEVICE_FLAG_DMA_RX);

    if ( res != RT_EOK )
    {
#if DEBUG
           rt_kprintf("error %d \n", res);
#endif
    }

    rt_sem_init(&rx_sem1, "rx_sem1", 0, RT_IPC_FLAG_FIFO);   //初始化信号量

    rt_device_set_rx_indicate(serialuart2 , uart2_input);  //挂载回调函数

    /*创建线程处理接收数据*/
    uart2_thread =rt_thread_create( "uart2",
                                      uart2_thread_entry,
                                      RT_NULL,
                                      THREAD_STACK_SIZE,
                                      THREAD_PRIORITY,
                                      THREAD_TIMESLICE);

   if (uart2_thread != RT_NULL)
        rt_thread_startup(uart2_thread);
    else
        return RT_ERROR;
   return RT_EOK;
}
INIT_APP_EXPORT(usart_rcv_start);
