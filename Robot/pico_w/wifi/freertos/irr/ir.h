#ifndef ir_h
#define ir_h

void set_side_arr(int side, int data);
int *get_side_arr();
void detectLines();
QueueHandle_t IRMessageHandler();
#endif