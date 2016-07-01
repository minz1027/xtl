/** @file
General purpose socket
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
@defgroup Sockets
@{
*/


#pragma once

#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
  static_assert(_WIN32_WINNT >= 0x600, "unsupported target Windows version");
  #define poll WSAPoll
#endif

#if (XTD_OS_MINGW & XTD_OS)
  #define POLLFD WSAPOLLFD
#endif

#if (XTD_COMPILER_IS_MSVC)
  #pragma comment(lib, "ws2_32")
#endif

namespace xtd{

  /** @namespace xtd::socket
  Declarations and definitions of the socket library
  */
  namespace socket{
///   @defgroup Sockets

#if ((XTD_OS_UNIX | XTD_OS_CYGWIN) & XTD_OS)
    using SOCKET = int;
  #define closesocket close
  #define ioctlsocket ioctl
#elif ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    using POLLFD = pollfd;
#endif
    ///Represents an socket error
    struct exception : xtd::os_exception{
      exception(const source_location& loc, const xtd::string& swhat) : os_exception(loc, swhat){}
      exception(const exception& ex) : os_exception(ex){}
      explicit exception(exception&& ex) : os_exception(std::move(ex)){}
    };

    ///IPv4 address wrapper around sockaddr_in
    struct ipv4address : sockaddr_in{
      static const int address_family = AF_INET;
      ipv4address(const char * sIP, uint16_t iPort){
        sin_family = AF_INET;
        sin_addr.s_addr = inet_addr(sIP);
        sin_port = htons(iPort);
      }
    };

    ///IPv6 address wrapper around sockaddr_in6
    struct ipv6address : sockaddr_in6{
      static const int address_family = AF_INET6;
      TODO("Implement proper ipv6 address");
    };

    ///@enum socket_type socket communication styles
    enum class socket_type{
      ///@var stream reliable FIFO stream on a remote socket
      stream = SOCK_STREAM,
      ///@var datagram unreliable connectionless broadcast
      datagram = SOCK_DGRAM,
      ///@var raw interface level data
      raw = SOCK_RAW,
    };

    ///@enum socket_protocol IP based protocols
    enum class socket_protocol{
      ///@var ip Internet Protocol v4
      ip = IPPROTO_IP,
      ///@var icmp Internet Control Message Protocol
      icmp = IPPROTO_ICMP,
      ///@var tcp Transmission Control Protocol
      tcp = IPPROTO_TCP,
      ///@var udp User Datagram Protocol
      udp = IPPROTO_UDP,
      ///@var ipv6 Internet Protocol v6
    };
    NOTE("add more socket protocols as needed defined in netinet/in.h")


    TODO("Refactor out socket::serializer into something more universal")
    ///Serializes data on a socket
    template <typename> struct serializer;

#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    /** Initializes WinSock
    Winsock requires a call to WSAStartup before any other calls to the winsock library.
    winsock_initializer is a global static that ensures winsock is initialized and cleaned up properly
    */
    struct winsock_initializer sealed{
      //singleton getter method
      static winsock_initializer &get(){
        static winsock_initializer oInit;
        return oInit;
      }
      winsock_initializer(const winsock_initializer&) = delete;
      winsock_initializer& operator=(const winsock_initializer&) = delete;
    private:
      winsock_initializer(){
        WSADATA oData;
        os_exception::throw_if(WSAStartup(0x202, &oData), [](int err){ return 0 != err; });
        if (oData.wVersion != 0x202) throw os_exception(here(), "Invalid winsock version");
      }

      ~winsock_initializer(){ WSACleanup(); }
    };
#endif


#if (!DOXY_INVOKED)
    template<typename _AddressT, socket_type, socket_protocol, template<class> class ... _Policies> struct socket_base;


    template<typename _AddressT, socket_type _ST, socket_protocol _PR, template<class> class _HeadT, template<class> class ..._TailT>
    struct socket_base<_AddressT, _ST, _PR, _HeadT, _TailT...> : _HeadT<socket_base<_AddressT, _ST, _PR, _TailT...> >{
      template<typename ... _ArgsT> explicit socket_base(_ArgsT &&...oArgs) : _HeadT<socket_base<_AddressT, _ST, _PR, _TailT...> >(std::forward<_ArgsT>(oArgs)...){}
    };
#endif

    /** Base class of the various socket specializations
    Hierarchy generation TMP pattern to create all the socket object types with the various behavioral policies.
     */
    template<typename _AddressT, socket_type _SocketT, socket_protocol _Protocol>
    struct socket_base<_AddressT, _SocketT, _Protocol>{

      using unique_ptr = std::unique_ptr<socket_base>;
      using shared_ptr = std::shared_ptr<socket_base>;
      using address_type = _AddressT;

      static const socket_type type = _SocketT;
      static const socket_protocol protocol = _Protocol;

      ~socket_base(){ close(); }

      socket_base() : _Socket(0){
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
        winsock_initializer::get();
#endif
        _Socket = xtd::os_exception::throw_if(::socket(address_type::address_family, (int)type, (int)protocol), [](SOCKET s){ return static_cast<SOCKET>(-1) == s; });
      }

      explicit socket_base(socket_base&& src) : _Socket(src._Socket){
        src._Socket = 0;
      }


      explicit socket_base(SOCKET newval) : _Socket(newval){}

      socket_base(const socket_base&) = delete;

      socket_base& operator=(const socket_base&) = delete;

      socket_base& operator=(socket_base&& src){
        std::swap(_Socket, src._Socket);
        return *this;
      }

      operator SOCKET() const{ return _Socket; }


      template <typename _Ty> void write(const _Ty& data){
        serializer<_Ty>::write(*this, data);
      }

      template <typename _Ty> void read(_Ty& data){
        serializer<_Ty>::read(*this, data);
      }

      template <typename _Ty> _Ty read(){
        _Ty data;
        serializer<_Ty>::read(*this, data);
        return data;
      }

      void close(){
        if (!_Socket){
          return;
        }
        ::closesocket(_Socket);
        _Socket = 0;
      }

      void set_blocking(bool blocking){
        uint32_t val = (blocking ? 0 : 1);
        xtd::os_exception::throw_if(ioctlsocket(_Socket, FIONBIO, &val), [](int i){ return i < 0; });
      }

    protected:
      SOCKET _Socket;

      template <typename _ReturnT> _ReturnT get_option(int Option);
      template <typename _ValueT> void set_option(_ValueT, int Option);
    };


    ///Polling behavior policy
    template <typename _SuperT>
    struct polling_socket : _SuperT{
      callback<void()> read_event;
      callback<void()> write_event;
      callback<void()> disconnect_event;
      callback<void()> error_event;
      void poll(int Timeout){
        struct pollfd oPoll;
        oPoll.events = POLLIN | POLLOUT;
        oPoll.fd = _SuperT::_Socket;
        oPoll.revents = 0;
        if (0 == xtd::os_exception::throw_if(::poll(&oPoll, 1, Timeout), [](int i){ return i < 0; })){
          return;
        }
        if (oPoll.revents & POLLERR){
          error_event();
        }
        if (oPoll.revents & POLLHUP){
          disconnect_event();
        }
        if (oPoll.revents & POLLIN){
          read_event();
        }
        if (oPoll.revents & POLLOUT){
          write_event();
        }
      }
      template<typename ... _ArgTs>
      explicit polling_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}
    private:

#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
      int poll(POLLFD *ufds, unsigned int nfds, int timeout){ return ::WSAPoll(ufds, nfds, timeout); }
#endif
    };


    ///Server side binding behavior
    template <typename _SuperT>
    struct bindable_socket : _SuperT{
      using address_type = typename _SuperT::address_type;

      template<typename ... _ArgTs>
      explicit bindable_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      void bind(const address_type& addr){
        exception::throw_if(::bind(_SuperT::_Socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(address_type)), [](int i){ return i < 0; });
      }
    };


    ///Client side connecting behavior
    template <typename _SuperT>
    struct connectable_socket : _SuperT{
      using address_type = typename _SuperT::address_type;

      template<typename ... _ArgTs>
      explicit connectable_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      void connect(const address_type& addr){
        exception::throw_if(::connect(_SuperT::_Socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(address_type)), [](int i){ return i < 0; });
      }
    };


    ///Server side listening behavior
    template <typename _SuperT>
    struct listening_socket : _SuperT{
      using address_type = typename _SuperT::address_type;

      template<typename ... _ArgTs>
      explicit listening_socket(_ArgTs &&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      void listen(int Backlog = SOMAXCONN){
        exception::throw_if(::listen(_SuperT::_Socket, Backlog), [](int i){ return i < 0; });
      }

      template <typename _ReturnT>
      _ReturnT accept(){
        return _ReturnT(exception::throw_if(::accept(_SuperT::_Socket, nullptr, nullptr), [](SOCKET s){ return (s <= 0); }));
      }
    };


    template <typename _SuperT>
    struct ip_options : _SuperT{
      using address_type = typename _SuperT::address_type;

      template<typename ... _ArgTs>
      explicit ip_options(_ArgTs&&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}
    };

    ///Async IO select behavior
    template <typename _SuperT>
    struct selectable_socket : _SuperT{
      template<typename ... _ArgTs>
      explicit selectable_socket(_ArgTs&&...oArgs) : _SuperT(std::forward<_ArgTs>(oArgs)...){}

      void select(int WaitMS, std::function<void()> onRead, std::function<void()> onWrite, std::function<void()> onError){
        timeval tv;
        fd_set fdRead, fdWrite, fdErr;
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdErr);
        FD_SET((SOCKET)*this, &fdRead);
        FD_SET((SOCKET)*this, &fdWrite);
        FD_SET((SOCKET)*this, &fdErr);
        tv.tv_sec = WaitMS / 1000;
        WaitMS %= 1000;
        tv.tv_usec = WaitMS / 1000;
#if (XTD_OS_MINGW & XTD_OS)
        auto iRet = xtd::os_exception::throw_if(::select(1 + (SOCKET)*this, &fdRead, &fdWrite, &fdErr, reinterpret_cast<PTIMEVAL>(&tv)), [](int i){return i < 0; });
#else
        auto iRet = xtd::os_exception::throw_if(::select(1 + (SOCKET)*this, &fdRead, &fdWrite, &fdErr, &tv), [](int i){return i < 0; });
#endif
        if (0 == iRet){
          return;
        }
        if (FD_ISSET((SOCKET)*this, &fdErr)){
          onError();
        }
        if (FD_ISSET((SOCKET)*this, &fdRead)){
          onRead();
        }
        if (FD_ISSET((SOCKET)*this, &fdWrite)){
          onWrite();
        }
      }
    };

#if (!DOXY_INVOKED)
    TODO("Get rid of these")
    template <typename _Ty>
    struct serializer{

      template <typename _SocketT>
      static void write(_SocketT& oSocket, const _Ty& src){
        static_assert(std::is_pod<_Ty>::value, "no acceptable specialization for type");
        exception::throw_if(::send(oSocket, reinterpret_cast<const char*>(&src), sizeof(_Ty), 0), [](int i){ return i <= 0; });
      }

      template <typename _SocketT>
      static void read(_SocketT& oSocket, _Ty& src){
        static_assert(std::is_pod<_Ty>::value, "no acceptable specialization for type");
        exception::throw_if(::recv(oSocket, reinterpret_cast<char*>(&src), sizeof(_Ty), 0), [](int i){ return i <= 0; });
      }
    };


    template <typename _Ty>
    struct NON_POD_Vector_Serializer{

      template <typename _SocketT>
      static void write(_SocketT& oSocket, const std::vector<_Ty>& src){
        serializer<typename std::vector<_Ty>::size_type>::write(oSocket, src.size());
        for (const auto & oItem : src){
          serializer<_Ty>::write(oSocket, oItem);
        }
      }

      template <typename _SocketT>
      static void read(_SocketT& oSocket, std::vector<_Ty>& src){
        typename std::vector<_Ty>::size_type count;
        serializer<typename std::vector<_Ty>::size_type>::read(oSocket, count);
        for (; count; --count){
          _Ty newval;
          serializer<_Ty>::read(oSocket, newval);
          src.push_back(newval);
        }
      }
    };


    template <typename _Ty>
    struct POD_Vector_Serializer{

      template <typename _SocketT >
      static void write(_SocketT& oSocket, const std::vector<_Ty>& src){
        serializer<typename std::vector<_Ty>::size_type>::write(oSocket, src.size());
        send(oSocket, reinterpret_cast<const char*>(&src[0]), (int)(sizeof(_Ty) * src.size()), 0);
      }

      template <typename _SocketT>
      static void read(_SocketT& oSocket, std::vector<_Ty>& src){
        typename std::vector<_Ty>::size_type count;
        serializer<typename std::vector<_Ty>::size_type>::read(oSocket, count);
        src.resize(count);
        recv(oSocket, reinterpret_cast<char*>(&src[0]), (int)(sizeof(_Ty) * count), 0);
      }

    };


    template <typename _Ty>
    struct serializer<std::vector<_Ty>> : std::conditional<std::is_pod<_Ty>::value, POD_Vector_Serializer<_Ty>, NON_POD_Vector_Serializer<_Ty>>::type{};
#endif

    using ipv4_tcp_stream = socket_base<ipv4address, socket_type::stream, socket_protocol::tcp, ip_options, connectable_socket, bindable_socket, listening_socket, selectable_socket>;
    using ipv4_udp_socket = socket_base<ipv4address, socket_type::datagram, socket_protocol::udp, ip_options>;

  }

}
