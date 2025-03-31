#ifndef __ir_H
#define __ir_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

extern volatile uint8_t IRTXBusy;
extern volatile void (*symbol_func)(void);

void IRStop();
void IRTX(const uint8_t * data, const protocol_t protocol, const uint32_t length, const uint32_t rpt, const uint32_t delay);

#ifdef __cplusplus
}
#endif
#endif /* __ir_H */
