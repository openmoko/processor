#ifndef UTIL_H
#define UTIL_H

#define psr_error(fmt...) do { \
    fprintf("%s:%d:error:", __FILE__, __LINE__); \
    fprintf(##fmt); \
    } while(0);

#define psr_warn(fmt...) do { \
    fprintf("%s:%d:warning:", __FILE__, __LINE__); \
    fprintf(##fmt); \
    } while(0);

#define psr_note(fmt...) do { \
    fprintf("%s:%d:note:", __FILE__, __LINE__); \
    fprintf(##fmt); \
    } while(0);

#define psr_warn(fmt...) do { \
    fprintf("%s:%d:debug:", __FILE__, __LINE__); \
    fprintf(##fmt); \
    } while(0);

#endif /* UTIL_H */
