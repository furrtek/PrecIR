#ifndef __init_H
#define __init_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

void Init(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);

#ifdef __cplusplus
}
#endif
#endif /* __init_H */
