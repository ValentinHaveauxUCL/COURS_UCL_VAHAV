#ifndef __MACROS_H_
#define __MACROS_H_


#define _LOG(prefix, msg, ...)\
do {\
fprintf(stderr, prefix "%s.%d: " msg "\n",\
__FILE__, __LINE__, ##__VA_ARGS__);\
} while(0)


#define LOG(msg, ...) _LOG("", msg, ##__VA_ARGS__)
#define ERROR(msg, ...) _LOG("[ERROR]", msg, ##__VA_ARGS__)



#endif /* __MACROS_H_ */
