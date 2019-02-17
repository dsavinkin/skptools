#include "drill.h"
#include <stdio.h>

extern "C"
{

typedef struct {
    DRILL_T dr;
    size_t amount;
} DRILL_ITEM_T;

static ARRAY_T drarray;

static bool drill_params_cmp_fn(void *element, void *data)
{
    DRILL_ITEM_T *item_ptr = (DRILL_ITEM_T *)element;
    DRILL_ITEM_T *new_item_ptr = (DRILL_ITEM_T *)data;

    //printf("compare d=%.1f to d=%.1f\n", item_ptr->dr.d, new_item_ptr->dr.d);

    if (((item_ptr->dr.side == SIDE_FRONT) || (item_ptr->dr.side == SIDE_BACK)) &&
        ((new_item_ptr->dr.side != SIDE_FRONT) && (new_item_ptr->dr.side != SIDE_BACK)))
    {
        return false;
    }

    if (((new_item_ptr->dr.side == SIDE_FRONT) || (new_item_ptr->dr.side == SIDE_BACK)) &&
        ((item_ptr->dr.side != SIDE_FRONT) && (item_ptr->dr.side != SIDE_BACK)))
    {
        return false;
    }

    //Compare diameter first
    if (item_ptr->dr.d == new_item_ptr->dr.d)
    {
        if ((item_ptr->dr.tdepth == 0) && (new_item_ptr->dr.tdepth == 0))
        {
            //return if depth is equal and both tdepth are zero
            return (item_ptr->dr.depth == new_item_ptr->dr.depth);
        }
        else
        {
            //return if tdepth is non zero and equal
            return (item_ptr->dr.tdepth == new_item_ptr->dr.tdepth);
        }
    }

    return false;
}

void drill_init(void)
{
    array_init(&drarray, sizeof(DRILL_ITEM_T));
}

void drill_append(const DRILL_T *dr, size_t amount)
{
    DRILL_ITEM_T item;
    DRILL_ITEM_T *item_ptr;
    item.dr = *dr;
    item.amount = amount;

    item_ptr = (DRILL_ITEM_T *)array_find_element(&drarray, drill_params_cmp_fn, &item);
    if (item_ptr)
    {
        item_ptr->amount += amount;
    }
    else
    {
        printf("insert d=%.1f, depth=%.1f, tdepth=%.1f\n", item.dr.d, item.dr.depth, item.dr.tdepth);
        array_insert(&drarray, &item);
    }
}

void drill_print_stat(void)
{
    printf("array_get_count() = %zd\n", array_get_count(&drarray));
    size_t total_drill_cnt = 0;
    for (size_t i = 0; i < array_get_count(&drarray); i++)
    {
        DRILL_ITEM_T *item_ptr = (DRILL_ITEM_T *)array_get_element(&drarray, i);
        printf("drill_type %3zd: d=%.1f, depth=%.1f, tdepth=%.1f, amount=%zd\n", i, item_ptr->dr.d, item_ptr->dr.depth, item_ptr->dr.tdepth, item_ptr->amount);
        total_drill_cnt += item_ptr->amount;
    }

    printf("total drill count: %zd\n", total_drill_cnt);
}

void drill_deinit(void)
{
    array_free(&drarray);
}

} //extern "C"
