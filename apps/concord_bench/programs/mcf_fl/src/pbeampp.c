/**************************************************************************
PBEAMPP.C of ZIB optimizer MCF, SPEC version

This software was developed at ZIB Berlin. Maintenance and revisions 
solely on responsibility of Andreas Loebel

Dr. Andreas Loebel
Ortlerweg 29b, 12207 Berlin

Konrad-Zuse-Zentrum fuer Informationstechnik Berlin (ZIB)
Scientific Computing - Optimization
Takustr. 7, 14195 Berlin-Dahlem

Copyright (c) 1998-2000 ZIB.           
Copyright (c) 2000-2002 ZIB & Loebel.  
Copyright (c) 2003-2005 Andreas Loebel.
**************************************************************************/
/*  LAST EDIT: Sun Nov 21 16:22:04 2004 by Andreas Loebel (boss.local.de)  */
/*  $Id: pbeampp.c,v 1.10 2005/02/17 19:42:32 bzfloebe Exp $  */



#include "pbeampp.h"




#ifdef _PROTO_
int bea_is_dual_infeasible( arc_t *arc, cost_t red_cost )
#else
int bea_is_dual_infeasible( arc, red_cost )
    arc_t *arc;
    cost_t red_cost;
#endif
{
    return(    (red_cost < 0 && arc->ident == AT_LOWER)
            || (red_cost > 0 && arc->ident == AT_UPPER) );
}







// typedef struct basket
// {
//     arc_t *a;
//     cost_t cost;
//     cost_t abs_cost;
// } BASKET;

// static long basket_size;
// static BASKET basket[B+K+1];
// static BASKET *perm[B+K+1];



#ifdef _PROTO_
void sort_basket( long min, long max, BASKET** perm) 
#else
void sort_basket( min, max )
    long min, max;
#endif
{
    long l, r;
    cost_t cut;
    BASKET *xchange;

    l = min; r = max;

    cut = perm[ (long)( (l+r) / 2 ) ]->abs_cost;

    // #pragma clang loop unroll_count(1)
    do // count: 15491708
    {
        // #pragma clang loop unroll_count(16)
        while( perm[l]->abs_cost > cut ) // count: 39932632
            l++;
        // #pragma clang loop unroll_count(16)
        while( cut > perm[r]->abs_cost ) 
            r--;
            
        if( l < r )
        {
            xchange = perm[l];
            perm[l] = perm[r];
            perm[r] = xchange;
        }
        if( l <= r )
        {
            l++; r--;
        }

    }
    while( l <= r );

    if( min < r )
        sort_basket( min, r, perm);
    if( l < max && l <= B )
        sort_basket( l, max, perm); 
}






// static long nr_group;
// static long group_pos;


// static long initialize = 1;


// #ifdef _PROTO_
arc_t *primal_bea_mpp( long m,  arc_t *arcs, arc_t *stop_arcs, 
                    cost_t *red_cost_of_bea, 
                    long* nr_group, long* group_pos, long* initialize, long* basket_size, BASKET* basket, BASKET** perm) 
// #else
// arc_t *primal_bea_mpp( m, arcs, stop_arcs, red_cost_of_bea )
//     long m;
//     arc_t *arcs;
//     arc_t *stop_arcs;
//     cost_t *red_cost_of_bea;
// #endif
{
    long i, next, old_group_pos;
    arc_t *arc;
    cost_t red_cost;

    if( *initialize )
    {
        // #pragma clang loop unroll_count(2) 
        for( i=1; i < K+B+1; i++ ) // count: -
            perm[i] = &(basket[i]);
        *nr_group = ( (m-1) / K ) + 1;
        *group_pos = 0;
        *basket_size = 0;
        *initialize = 0;
    }
    else
    {   
        // #pragma clang loop unroll_count(1) // count: 5694586
        for( i = 2, next = 0; i <= B && i <= *basket_size; i++ )
        {
            arc = perm[i]->a;
            red_cost = arc->cost - arc->tail->potential + arc->head->potential;
            if( (red_cost < 0 && arc->ident == AT_LOWER)
                || (red_cost > 0 && arc->ident == AT_UPPER) )
            {
                next++;
                perm[next]->a = arc;
                perm[next]->cost = red_cost;
                perm[next]->abs_cost = ABS(red_cost);
            }
                }   
        *basket_size = next;
        }

    old_group_pos = *group_pos;

NEXT:
    /* price next group */
    arc = arcs + *group_pos;
    // #pragma clang loop unroll_count(4)
    for( ; arc < stop_arcs; arc += *nr_group ) // count: 82002022
    {
        if( arc->ident > BASIC )
        {
            /* red_cost = bea_compute_red_cost( arc ); */
            red_cost = arc->cost - arc->tail->potential + arc->head->potential;
            if( bea_is_dual_infeasible( arc, red_cost ) )
            {
                (*basket_size)++;
                // printf("basket_size: %ld\n", *basket_size);
                perm[*basket_size]->a = arc;
                perm[*basket_size]->cost = red_cost;
                perm[*basket_size]->abs_cost = ABS(red_cost);
            }
        }
        
    }

    if( ++(*group_pos) == *nr_group )
        *group_pos = 0;

    if( *basket_size < B && *group_pos != old_group_pos )
        goto NEXT;

    if( *basket_size == 0 )
    {
        *initialize = 1;
        *red_cost_of_bea = 0; 
        return NULL;
    }
    
    sort_basket( 1, *basket_size, perm);
    
    *red_cost_of_bea = perm[1]->cost;
    return( perm[1]->a );
}










