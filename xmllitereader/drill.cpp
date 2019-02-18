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

//TODO: add drills to joint one by one or define set of joints
static JOINT_T *_add_joint(char *name, DRILL_CLASS_T *dc1, DRILL_CLASS_T *dc2,
                           bool side, double distmin, double distmax, double zmin, double zmax)
{
    JOINT_T joint;
    joint.name = name;
    joint.dc1 = dc1;
    joint.dc2 = dc2;
    joint.side = side;
    joint.distmin = distmin;
    joint.distmax = distmax;
    joint.zmin = zmin;
    joint.zmax = zmax;

    return (JOINT_T*)array_insert(&joints, &joint);
}

static void _joints_init_values(void)
{
    DRILL_CLASS_T *dc[10];

    dc[0] = _add_drill_class("Konfirmat 6.4 head", 6.4, 7.0, 2, 21, true);
    //TODO: can we check the sum of thickness + depth?
    dc[1] = _add_drill_class("Konfirmat x50 thread", 4.4, 5.0, 33, 50, false);
    _add_joint("Konfirmat 6.4/50", dc[0], dc[1], false, 0, 0, 0, 0);

    dc[0] = _add_drill_class("dowel face", 8.0, 8.0, 12, 15, false);
    dc[1] = _add_drill_class("dowel 8x30 side", 8.0, 8.0, 20, 21, false);
    dc[2] = _add_drill_class("dowel 8x35 side", 8.0, 8.0, 25, 26, false);
    dc[3] = _add_drill_class("dowel 8x60 face", 8.0, 8.0, 16, 21, true);
    _add_joint("dowel 8x30", dc[0], dc[1], false, 0, 0, 0, 0);
    _add_joint("dowel 8x35", dc[0], dc[2], false, 0, 0, 0, 0);
    _add_joint("dowel 8x60", dc[3], dc[2], false, 0, 0, 0, 0);

    dc[0] = _add_drill_class("rafix VB 35/18 drill", 20, 20, 14, 18, false);
    dc[1] = _add_drill_class("rafix VB 35/16 drill", 20, 20, 12.5, 18, false);
    dc[2] = _add_drill_class("rafix VB 135/16 drill", 20, 20, 12.5, 18, false);
    dc[3] = _add_drill_class("for rafix DU 321", 5, 5, 11.5, 18, false);
    _add_joint("rafix VB 35/18", dc[0], dc[3], true, 9.5, 10, 9, 9.5);
    _add_joint("rafix VB 35/16", dc[1], dc[3], true, 9.5, 10, 7.5, 8);
    _add_joint("rafix VB 135/16", dc[2], dc[3], true, 9.5, 10, 8, 8);

    dc[0] = _add_drill_class("DU 232 30mm", 5, 5, 11.5, 18, false);
    dc[1] = _add_drill_class("DU 853 49/30mm", 8, 8, 18, 19, true);
    dc[2] = _add_drill_class("DU 868 46/30mm", 8, 8, 16, 16, true);

    //TODO: fit this drill with dc4..7
    dc[3] = _add_drill_class("Rastex side", 8.0, 8.0, 28, 36, false);
    dc[4] = _add_drill_class("Rastex 15/12", 15, 15, 10, 12.6, false);
    dc[5] = _add_drill_class("Rastex 15/15", 15, 15, 12.7, 13.3, false);
    dc[6] = _add_drill_class("Rastex 15/18", 15, 15, 13.4, 15.6, false);
    dc[7] = _add_drill_class("Rastex 15/22", 15, 15, 15.7, 19, false);

    _add_joint("Rastex 15/12 + DU 232", dc[0], dc[4], true, 34, 34, 6, 6);
    _add_joint("Rastex 15/15 + DU 232", dc[1], dc[4], true, 34, 34, 8, 8);
    _add_joint("Rastex 15/18 + DU 232", dc[2], dc[4], true, 34, 34, 9, 9);
    _add_joint("Rastex 15/22 + DU 232", dc[3], dc[4], true, 34, 34, 11, 11);

}

static bool drill_class_cmp_fn(void *element, void *data)
{
    DRILL_CLASS_T *dc = (DRILL_CLASS_T*)element;
    DRILL_T *dr = (DRILL_T*)data;

    if (!dc || !dr)
    {
        return false;
    }

    if ((dr->d < dc->dmin) || (dr->d > dc->dmax))
    {
        return false;
    }

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

        printf("drill_type %3zd: d=%.1f, depth=%.1f, tdepth=%.1f, amount=%zd, '%s'\n",
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
