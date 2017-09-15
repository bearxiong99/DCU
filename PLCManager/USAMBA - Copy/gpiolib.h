#ifndef GPIOLIB_H_
#define GPIOLIB_H_

#define GPIO_IN  1
#define GPIO_OUT 0

int gpioexport(int gpioid);
int gpiosetdir(int gpioid,int mode);
int gpiogetbits(int gpioid);
int gpiosetbits(int gpioid);
int gpioclearbits(int gpioid);


#endif /* GPIOLIB_H_ */
