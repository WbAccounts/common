/*
 *tickserver.hpp: 2023-01-06 created by qudreams
 *Copyright to qianxin enterprise security group
 *
 *tick control center server to get local communication ip
 */

 #include <sys/select.h>
 #include <sys/socket.h>
 #include <sys/time.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include "log/log.h"

namespace TickServer {
    static inline int TryConnect(int fd,const struct sockaddr* addr,socklen_t addrlen)
    {
    	int rc = 0;
    	int err = 0;
    	int flags = 0;

    	fcntl(fd,F_GETFL,&flags);
    	fcntl(fd,F_SETFL,flags | O_NONBLOCK);

    	rc = ::connect(fd,addr,addrlen);
    	if(rc == 0) { return rc; }

    	socklen_t err_len = sizeof(err);
    	err = errno;//此处不要调用getsockopt，因为此时errno的值是被置位的，但getsockopt无法获取相应的值
    	if(err != EINPROGRESS) {
    		LOG_ERROR("TryConnect: failed to connect server,"
    					"because: %s",strerror(errno));
    		return -1;
    	}

    	fd_set fds;
    	FD_ZERO(&fds);
    	FD_SET(fd,&fds);

    	struct timeval tv = {5,0}; //5 seconds
    	rc = select(fd + 1,NULL,&fds,NULL,&tv);
    	if(rc == 0) {
    		//timeout
    		rc = -1;
    		LOG_ERROR("TryConnect: connect server timedout");
    	} else if(rc < 0) {
    		err = errno;
    		LOG_ERROR("TryConnect: connect server error,because select() error: %s"
    			,strerror(err));
    	} else if(rc > 0) {
    		rc = err = 0;
    		err_len = sizeof(err);
    		//此处rc > 0只是表示select返回成功，存在可写的套接字，但连接失败时，套接字是即可读又可写，
    		//所以此时要调用getsockopt获取套接字上是否存错误，根据此错误值来判断连接是否成功
    		rc = getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&err_len);
    		if(rc != 0) {
    			LOG_ERROR("TryConnect: failed to get socket error,because %s"
    					,strerror(errno));
    			return rc;
    		}

    		if(err != 0) {
    			LOG_ERROR("TryConnect: connect server error,because %s"
    				,strerror(err));
    			rc = -1;
    		}
    	}

    	return rc;
    }


    /*
     *连接服务端并返回连接时本地使用的ip地址
     *@family: AF_INET/AF_INET6
     *@addrbuf: 指向struct in_addr/struct in6_addr结构
     *@serverport: 服务器服务端口，主机字节序
     *@return: 成功返回0,失败返回-1
     */
    static inline int TryTickServer(int family,void* addrbuf,
                    int serverport,std::string& localPeerIP)
    {
    	int rc = -1;
    	int sockfd = -1;
    	socklen_t len = 0;
    	char ip[256] = {0};
    	void* pinaddr = NULL;
    	struct sockaddr_in addr4;
    	struct sockaddr_in6 addr6;
    	struct sockaddr* paddr = NULL;

    	if(family == AF_INET) {
    		addr4.sin_family = AF_INET;
    		addr4.sin_port = htons(serverport & 0xFFFF);
    		memcpy(&addr4.sin_addr,addrbuf,sizeof(addr4.sin_addr));
    		paddr = (struct sockaddr*)&addr4;
    		len = sizeof(addr4);
    	} else {
    		addr6.sin6_family = AF_INET6;
    		addr6.sin6_port = htons(serverport & 0xFFFF);;
    		memcpy(&addr6.sin6_addr,addrbuf,sizeof(addr6.sin6_addr));
    		paddr = (struct sockaddr*)&addr6;
    		len = sizeof(addr6);
    	}

    	sockfd = socket(family,SOCK_STREAM,0);
    	if(sockfd == -1) {
    		LOG_ERROR("TryTickServer: failed to create socket,"
    					"because: %s",strerror(errno));
    		return rc;
    	}

     	rc = TickServer::TryConnect(sockfd,paddr,len);
    	if(rc == -1) { close(sockfd); return rc; }

    	//connect ok,now we try to get local peer ip&port
    	::bzero(paddr,len);
    	rc = getsockname(sockfd,paddr,&len);
    	if(rc == -1) { close(sockfd); return rc; }

    	if(paddr->sa_family == AF_INET) {
    		pinaddr = &(((struct sockaddr_in*)paddr)->sin_addr);
    	} else {
    		pinaddr = &(((struct sockaddr_in6*)paddr)->sin6_addr);
    	}

    	inet_ntop(paddr->sa_family,pinaddr,ip,sizeof(ip));
    	close(sockfd);

        localPeerIP = ip;

    	return rc;
    }
}
