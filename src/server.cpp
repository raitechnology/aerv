#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <aekv/ev_aeron.h>
#include <sassrv/ev_rv.h>
#include <raikv/mainloop.h>

extern "C" const char *aeron_version_full();

using namespace rai;
using namespace aekv;
using namespace sassrv;
using namespace kv;

struct Args : public MainLoopVars { /* argv[] parsed args */
  int rv_port;
  Args() : rv_port( 0 ) {}
};

struct MyListener : public EvRvListen, public EvAeron {
  char      aeron_pub[ 128 ],
            aeron_sub[ 128 ];
  void * operator new( size_t, void *ptr ) { return ptr; }
  MyListener( kv::EvPoll &p ) : EvRvListen( p ), EvAeron( p ) {}

  virtual int start_host( void ) noexcept final {
    uint8_t * rcv = (uint8_t *) (void *) &this->mcast.recv_ip[ 0 ],
            * snd = (uint8_t *) (void *) &this->mcast.send_ip,
            * hst = (uint8_t *) (void *) &this->mcast.host_ip;
    if ( hst[ 0 ] != 127 ) {
      if ( this->EvAeron::context != NULL ) {
        fprintf( stderr, "Not shutdown yet!\n" );
        return -1;
      }
      snprintf( this->aeron_sub, sizeof( this->aeron_sub ),
        "aeron:udp?endpoint=%u.%u.%u.%u:%s|interface=%u.%u.%u.%u",
        rcv[ 0 ], rcv[ 1 ], rcv[ 2 ], rcv[ 3 ], this->service,
        hst[ 0 ], hst[ 1 ], hst[ 2 ], hst[ 3 ] );
      snprintf( this->aeron_pub, sizeof( this->aeron_pub ),
        "aeron:udp?endpoint=%u.%u.%u.%u:%s|interface=%u.%u.%u.%u",
        snd[ 0 ], snd[ 1 ], snd[ 2 ], snd[ 3 ], this->service,
        hst[ 0 ], hst[ 1 ], hst[ 2 ], hst[ 3 ] );

      printf( "start_network:        service %.*s, \"%.*s\"\n",
              (int) this->service_len, this->service, (int) this->network_len,
              this->network );
      printf( "aeron_sub:            %s\n", this->aeron_sub );
      printf( "aeron_pub:            %s\n", this->aeron_pub );

      AeronSvcId id;
      id.pub_if  = this->mcast.host_ip;
      id.sub_if  = this->mcast.host_ip;
      id.pub_svc = this->service_port;
      id.sub_svc = this->service_port;
      if ( ! this->EvAeron::start_aeron( &id, this->aeron_pub, 1001,
                                         this->aeron_sub, 1001 ) )
        return -1;
      return 0;
    }
    return this->EvRvListen::start_host();
  }
  virtual void on_connect( void ) noexcept final {
    this->EvRvListen::start_host();
  }
  virtual int stop_host( void ) noexcept final {
    printf( "stop_network:         service %.*s, \"%.*s\"\n",
            (int) this->service_len, this->service, (int) this->network_len,
            this->network );
    this->EvRvListen::stop_host();
    this->EvAeron::do_shutdown();
    return 0;
  }
};

struct Loop : public MainLoop<Args> {
  Loop( EvShm &m,  Args &args,  int num, bool (*ini)( void * ) ) :
    MainLoop<Args>( m, args, num, ini ) {}

 MyListener * rv_sv;

  bool rv_init( void ) {
    return Listen<MyListener>( 0, this->r.rv_port, this->rv_sv,
                               this->r.tcp_opts );
  }
  bool init( void ) {
    printf( "aeron_version_full:   %s\n", aeron_version_full() );
    printf( "aerv_version:         " kv_stringify( AERV_VER ) "\n" );
    printf( "rv_daemon:            %d\n", this->r.rv_port );
    return this->rv_init();
  }
  static bool initialize( void *me ) noexcept {
    return ((Loop *) me)->init();
  }
};


int
main( int argc, const char *argv[] )
{
  EvShm shm;
  Args  r;

  r.no_threads   = true;
  r.no_reuseport = true;
  r.no_map       = true;
  r.no_default   = true;
  r.all          = true;
  r.add_desc( "  -r rv    = listen rv port        (7500)" );
  if ( ! r.parse_args( argc, argv ) )
    return 1;
  if ( shm.open( r.map_name, r.db_num ) != 0 )
    return 1;
  shm.print();
  r.rv_port = r.parse_port( argc, argv, "-r", "7500" );
  Runner<Args, Loop> runner( r, shm, Loop::initialize );
  if ( r.thr_error == 0 )
    return 0;
  return 1;
}

