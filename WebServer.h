/*
 * WebServer.h
 *
 * Created: 7.8.2014 0:38:00
 *  Author: Daniel
 */ 


#ifndef WEBSERVER_H_
#define WEBSERVER_H_
char WebServerEnabled;
void WebServerLoop();
void HandleEstabilishedConnection();

unsigned char Requestheadercache[64];



#endif /* WEBSERVER_H_ */