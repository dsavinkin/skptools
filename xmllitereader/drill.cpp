#include "drill.h"
#include <stdio.h>

extern "C"
{

/***************************************************************/
/*                    Local types                              */
/***************************************************************/

typedef struct {
    DRILL_T dr;
    size_t amount;
} DRILL_ITEM_T;

/***************************************************************/
/*                    Local variables                          */
/***************************************************************/


static ARRAY_T drills;
static ARRAY_T drill_classes;
static ARRAY_T joints;

/***************************************************************/
/*                    Local functions                          */
/***************************************************************/

static DRILL_CLASS_T *_add_drill_class(char *name, double dmin, double dmax,
                                       double depthmin, double depthmax, bool through)
{
    DRILL_CLASS_T dc;
    dc.name = name;
    dc.dmin = dmin;
    dc.dmax = dmax;
    dc.depthmin = depthmin;
    dc.depthmax = depthmax;
    dc.through = through;

    return (DRILL_CLASS_T*)array_insert(&drill_classes, &dc);
}

static JOINT_T *_add_joint(char *name, DRILL_CLASS_T *dc1, DRILL_CLASS_T *dc2,
                           bool side, double distmin, double distmax)
{
    JOINT_T joint;
    joint.name = name;
    joint.dc1 = dc1;
    joint.dc2 = dc2;
    joint.side = side;
    joint.distmin = distmin;
    joint.distmax = distmax;

    return (JOINT_T*)array_insert(&joints, &joint);
}

static void _joints_init_values(void)
{
    DRILL_CLASS_T *dc1;
    DRILL_CLASS_T *dc2;
    JOINT_T *joint;

    dc1 = _add_drill_class("Konfirmat_head", 6.4, 7.0, 2, 21, true);
    //TODO: can we check the sum of thickness + depth?
    dc2 = _add_drill_class("Konfirmat_thread", 4.4, 5.0, 33, 50, false);
    joint = _add_joint("Konfirmat", dc1, dc2, false, 0, 0);
}

static bool drill_class_cmp_fn(void *element, void *data)
{
    DRILL_CLASS_T *dc = (DRILL_CLASS_T*)element;
    DRILL_T *dr = (DRILL_T*)data;

    if (!dc || !dr)
    {
        return false;
    }

    //printf("compare %.1f <= %.1f <= %.1f\n", dc->dmin, dr->d, dc->dmax);

    if ((dr->d < dc->dmin) || (dr->d > dc->dmax))
    {
        return false;
    }

    //printf("compare %d == (%.1f > 0)\n", dc->through, dr->tdepth);

    bool through = (dr->tdepth > 0);
    if (through != dc->through)
    {
        return false;
    }

    if (through)
    {
        if ((dr->tdepth < dc->depthmin) || (dr->tdepth > dc->depthmax))
        {
            return false;
        }
    }
    else
    {
        if ((dr->depth < dc->depthmin) || (dr->depth > dc->depthmax))
        {
            return false;
        }
    }

    return true;
}

static DRILL_CLASS_T *_drill_find_class(DRILL_T *dr)
{
    return (DRILL_CLASS_T *)array_find_element(&drill_classes, drill_class_cmp_fn, dr);
}

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

/***************************************************************/
/*                   Public functions                          */
/***************************************************************/

void drill_init(void)
{
    array_init(&drills, sizeof(DRILL_ITEM_T));
    array_init(&drill_classes, sizeof(DRILL_CLASS_T));
    array_init(&joints, sizeof(JOINT_T));

    _joints_init_values();
}

void drill_append(const DRILL_T *dr, size_t amount)
{
    DRILL_ITEM_T item;
    DRILL_ITEM_T *item_ptr;
    item.dr = *dr;
    item.amount = amount;

    item_ptr = (DRILL_ITEM_T *)array_find_element(&drills, drill_params_cmp_fn, &item);
    if (item_ptr)
    {
        item_ptr->amount += amount;
    }
    else
    {
        printf("insert d=%.1f, depth=%.1f, tdepth=%.1f\n", item.dr.d, item.dr.depth, item.dr.tdepth);
        array_insert(&drills, &item);
    }
}

void drill_print_stat(void)
{
    printf("array_get_count() = %zd\n", array_get_count(&drills));
    size_t total_drill_cnt = 0;
    for (size_t i = 0; i < array_get_count(&drills); i++)
    {
        DRILL_ITEM_T *item_ptr = (DRILL_ITEM_T *)array_get_element(&drills, i);

        DRILL_CLASS_T *dc = _drill_find_class(&(item_ptr->dr));

        printf("drill_type %3zd: d=%.1f, depth=%.1f, tdepth=%.1f, amount=%zd, class_name: %s\n",
               i, item_ptr->dr.d, item_ptr->dr.depth, item_ptr->dr.tdepth, item_ptr->amount, dc ? dc->name : "NONE");
        total_drill_cnt += item_ptr->amount;

    }

    printf("total drill count: %zd\n", total_drill_cnt);
}

void drill_deinit(void)
{
    array_free(&drills);
    array_free(&drill_classes);
    array_free(&joints);
}

} //extern "C"
