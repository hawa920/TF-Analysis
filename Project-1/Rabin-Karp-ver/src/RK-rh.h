#ifndef RK-RH_H
#define RK-RH_H


extern int TRUE;
extern int FALSE;
extern int RK_RHB;
extern int RK_RHM;

__attribute__((always_inline)) inline int myabs(const char *src);

__attribute__((always_inline)) inline long long get_hv(\
    const int l_ofst,\
    const int r_ofst,\
    const long long *prefix_ptr,\
    const int *basetab_ptr );

void ini_hash_factor(const int base, const int mod);





#endif
