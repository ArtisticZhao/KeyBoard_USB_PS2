#ifndef __UART_PROCESS_H_
#define __UART_PROCESS_H_

#include <stddef.h>

#define CMD_BUFSIZE 20
#define PRINT_BUFSIZE (CMD_BUFSIZE*3+1)

/**
 * @func 字节串 转HEX串
 * @args
 *    - param[in] in 输入字节串
 *    - param[in] size 字节串长度
 */
char* print_bytes(const char* in, size_t size);

/**
 * @func 解析ch9350串口输出的按键值
 * @args
 *    - param[in] in 输入字节串
 *    - param[in] size 字节串长度
 */
void ch9350_parser(const char* in, size_t size);

#endif
