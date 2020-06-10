#ifndef GUI_H
#define GUI_H

#include "stdlib.h"
#include "stdbool.h"

#define TABLE_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define GUI_NUM_LINES 4

typedef struct
{
    const char *label;
    void (*handler)(void);
}gui_entry_t;

typedef struct
{
    const char *label;
    void *value;
    const char *format;
    uint32_t upperBound;
    int32_t lowerBound;
}gui_settings_t;

void GUI_Init();
void GUI_Deinit();
bool GUI_is_inited();
void GUI_Update();
void MainHandler();
void TestsHandler();

void GUI_systick();
void GUI_extend_activity();
bool GUI_can_sleep();

void GUI_receive_complete(uint8_t * data, uint16_t length);



#endif
