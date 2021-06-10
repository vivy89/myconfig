#ifndef __MYDEBUG_H__
#define __MYDEBUG_H__

#define serr(...)	do{ \
	if (ret < 0) {	\
		perror("\033[31m"__VA_ARGS__"\033[0m"); \
		exit(-1);	\
	}	\
}while(0)
#define err(...)	do{ \
	if (ret < 0) {	\
		printf("\033[31merr:"__VA_ARGS__"\033[0m\n"); \
		exit(-1);	\
	}	\
}while(0)




#endif
