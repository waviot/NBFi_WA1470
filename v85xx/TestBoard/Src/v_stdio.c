/**
  * @file    v_stdio.h
  * @author  Application Team
  * @version V4.4.0
  * @date    2018-09-27
  * @brief   standard printf.
******************************************************************************/

#include "v_stdio.h"
#include "target.h"
#include <stdio.h>
#ifdef __GNUC__
  #include <unistd.h>
#endif /* __GNUC__ */

/**
  * @brief  printf init.
  * @param  None
  * @retval None
  */
void Stdio_Init(void)
{
  UART5->BAUDDIV = CLK_GetPCLKFreq()/115200;
  UART5->CTRL = UART_CTRL_TXEN;
}

#ifdef __GNUC__
int _write(int32_t fd, char* ptr, int32_t len)
{
  uint32_t i;

  if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
  {
    i = 0UL;
    while (i < len)
    {
      UART5->DATA = ptr[i++];
      while (!(UART5->STATE&UART_STATE_TXDONE));
      UART5->STATE = UART_STATE_TXDONE;
    }
  }
  return len;
}
#else
int fputc(int ch, FILE *f)
{
  UART5->DATA = ch;
  while (!(UART5->STATE&UART_STATE_TXDONE));
  UART5->STATE = UART_STATE_TXDONE;
  return ch;
}
#endif /* __GNUC__ */

/*********************************** END OF FILE ******************************/
