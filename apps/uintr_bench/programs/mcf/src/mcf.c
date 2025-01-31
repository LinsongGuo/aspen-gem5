/**************************************************************************
MCF.H of ZIB optimizer MCF, SPEC version

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
/*  LAST EDIT: Thu Feb 17 22:10:51 2005 by Andreas Loebel (boss.local.de)  */
/*  $Id: mcf.c,v 1.15 2005/02/17 21:43:12 bzfloebe Exp $  */



#include "mcf.h"
#include "programs.h"

// #define REPORT

// extern long min_impl_duration;






#ifdef _PROTO_
long global_opt( network_t* net )
#else
long global_opt( )
#endif
{
    long new_arcs;
    long residual_nb_it;
    

    new_arcs = -1;
    residual_nb_it = net->n_trips <= MAX_NB_TRIPS_FOR_SMALL_NET ?
        MAX_NB_ITERATIONS_SMALL_NET : MAX_NB_ITERATIONS_LARGE_NET;

    while( new_arcs )
    {
#ifdef REPORT
        printf( "active arcs                : %ld\n", net.m );
#endif

        primal_net_simplex( net );


#ifdef REPORT
        printf( "simplex iterations         : %ld\n", net.iterations );
        printf( "objective value            : %0.0f\n", flow_cost(&net) );
#endif


#if defined AT_HOME
        printf( "%ld residual iterations\n", residual_nb_it );
#endif

        if( !residual_nb_it )
            break;


        if( net->m_impl )
        {
          new_arcs = suspend_impl( net, (cost_t)-1, 0 );

#ifdef REPORT
          if( new_arcs )
            printf( "erased arcs                : %ld\n", new_arcs );
#endif
        }


        new_arcs = price_out_impl( net );

#ifdef REPORT
        if( new_arcs )
            printf( "new implicit arcs          : %ld\n", new_arcs );
#endif
        
        if( new_arcs < 0 )
        {
#ifdef REPORT
            printf( "not enough memory, exit(-1)\n" );
#endif

            exit(-1);
        }

// #ifndef REPORT
//         printf( "\n" );
// #endif


        residual_nb_it--;
    }

    // _stui();
    // printf( "checksum                   : %ld\n", net.checksum );
    // _clui();

    return  net->checksum;

    // return 0;
}


void mcf_init() {
//     //     _clui();
//     // printf("mcf stui\n");
//     memset( (void *)(&net), 0, (size_t)sizeof(network_t) );
//     net.bigM = (long)BIGM;

//     // strcpy( net.inputfile, argv[1] );
//     strcpy( net.inputfile, "/data/preempt/caladan-uintr/apps/uintr_bench/programs/mcf/data/test/input/inp.in");
    

//     if( read_min( &net ) )
//     {
//         printf( "read error, exit\n" );
//         getfree( &net );
//         return;
//     }
// //     printf("mcf clui\n");
// //     _stui();


// // #ifdef REPORT
// //     printf( "nodes                      : %ld\n", net.n_trips );
// // #endif

}




// #ifdef _PROTO_
// int main( int argc, char *argv[] )
// #else
// int main( argc, argv )
//     int argc;
//     char *argv[];
// #endif
long long mcf()
{    
    // if( argc < 2 )
    //     return -1;


    // printf( "\nMCF SPEC CPU2006 version 1.10\n" );
    // printf( "Copyright (c) 1998-2000 Zuse Institut Berlin (ZIB)\n" );
    // printf( "Copyright (c) 2000-2002 Andreas Loebel & ZIB\n" );
    // printf( "Copyright (c) 2003-2005 Andreas Loebel\n" );
    // printf( "\n" );


    _clui();
    network_t net;
    // printf("mcf stui\n");
    memset( (void *)(&net), 0, (size_t)sizeof(network_t) );
    net.bigM = (long)BIGM;

    // strcpy( net.inputfile, argv[1] );
    strcpy( net.inputfile, "/data/preempt/caladan-uintr/apps/uintr_bench/programs/mcf/data/test/input/inp.in");
    

    if( read_min( &net ) )
    {
        printf( "read error, exit\n" );
        getfree( &net );
        return -1;
    }
    // printf("mcf clui\n");
    _stui();


#ifdef REPORT
    printf( "nodes                      : %ld\n", net.n_trips );
#endif


    primal_start_artificial( &net );
    long checkout = global_opt( &net );


#ifdef REPORT
    printf( "done\n" );
#endif

    
    // _clui();
    // if( write_circulations( "mcf.out", &net ) )
    // {
    //     getfree( &net );
    //     _clui();
    //     return -1;    
    // }


    // getfree( &net );
    // _stui();
    return (long long) checkout;
}
