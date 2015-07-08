/*
 * sockets.h
 *
 * Created: 28.7.2014 23:08:28
 *  Author: Daniel
 */ 


#ifndef SOCKETS_H_
#define SOCKETS_H_


unsigned char			Socket_Open( unsigned char  eth_protocol, unsigned int  tcp_port);
void					Socket_Close();
void					Socket_Disconect();
unsigned char			Socket_Listen();
unsigned char			Socket_Write(const unsigned char  *buf, unsigned int  buflen);
unsigned int			Socket_Read(unsigned char  *buf, unsigned int  buflen);
unsigned int			Socket_Available();
#endif /* SOCKETS_H_ */