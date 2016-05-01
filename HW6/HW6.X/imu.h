#ifndef IMU_
#define IMU_

void init_OC(void);
void init_ctrl1(void);
void init_ctrl2(void);
void init_ctrl3(void);
void I2C_read_multipleLine(char address, char register_addr, short * data, char length);

#endif