#pragma once

/***************************************************************/
/*                     Global Definitions                      */
/***************************************************************/

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#define MM2INCH(x) ((x)/25.4)
#define INCH2MM(x) ((x)*25.4)

#ifndef SU_CALL
#define SU_CALL(func) if ((func) != SU_ERROR_NONE) { printf("Error on Line %d\n", __LINE__); throw std::exception(); }
#endif

#define PARSE_FAIL(ret)                do { printf("PARSE_FAIL line %d\n", __LINE__); return (ret); } while(0)

#define DISTANCE_X 50 //mm
#define DISTANCE_Y 50 //mm
#define DISTANCE_Z 3 //*thickness

#define SIDE_FRONT  0
#define SIDE_LEFT   1
#define SIDE_TOP    2
#define SIDE_RIGHT  3
#define SIDE_BOTTOM 4
#define SIDE_BACK   5

#define CORNER_LOWER_LEFT   0
#define CORNER_UPPER_LEFT   1
#define CORNER_UPPER_RIGHT  2
#define CORNER_LOWER_RIGHT  3
#define CORNER_MAX          4

#define EDGE_COVER_BOTH     0
#define EDGE_COVER_H        1
#define EDGE_COVER_V        2

/***************************************************************/
/*                       Global Types                          */
/***************************************************************/

typedef struct {
    unsigned char *array;
    size_t used;
    size_t size;
    size_t element_size;
} ARRAY_T;

/***************************************************************/
/*                  Function declarations                      */
/***************************************************************/

void array_init(ARRAY_T *a, size_t element_size);
void array_insert(ARRAY_T *a, void *element);
void *array_get_element(ARRAY_T *a, size_t pos);
size_t array_get_count(ARRAY_T *a);
void array_free(ARRAY_T *a);
