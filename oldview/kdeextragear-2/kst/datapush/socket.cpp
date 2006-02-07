/*
   Copyright (C) 2003 The University of Toronto

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool Socket::_init = false;

void Socket::init() {
  ::signal(SIGPIPE, SIG_IGN);
  _init = true;
}


Socket::Socket(struct sockaddr_in *peer, int fd)
: _client(peer), _fd(fd) {
}


Socket::Socket()
: _fd(-1) {
  if (!_init) {
    init();
  }

  _client = static_cast<struct sockaddr_in*>(::malloc(sizeof(struct sockaddr_in)));
}


Socket::~Socket() {
  close();
  if (_client) {
    ::free(_client);
    _client = 0L;
  }
}


void Socket::close() {
  if (_fd >= 0) {
    ::close(_fd);
    _fd = -1;
  }
}


int Socket::open(const char *addr, int port) {
  if (!addr || !*addr ||  port < 0 || !_client) {
    return -1;
  }

  if (_fd >= 0) {
    close();
  }

  ::memset(_client, 0, sizeof(_client));

  _fd = ::socket(PF_INET, SOCK_STREAM, 0);

  if (_fd < 0) {
    return -1;
  }

  _client->sin_family = AF_INET;

  if (0 > resolv(addr, port)) {
    close();
    return -1;
  }

  if (0 > ::connect(_fd, reinterpret_cast<struct sockaddr*>(_client), sizeof(struct sockaddr_in))) {
    close();
    return -1;
  }

  return 0;
}


int Socket::resolv(const char *addr, int port) {
  struct hostent he;
  struct hostent *phe = &he;
  int h_err = 0;
  char tmp[1025];

  if (0 > ::gethostbyname_r(addr, phe, tmp, 1024, &phe, &h_err)) {
    return -1;
  }

  if (0 == ::inet_aton(phe->h_addr, &(_client->sin_addr))) {
    return -1;
  }

  _client->sin_port = ntohs(port);
  return 0;
}


int Socket::write(const char *buf, int len) {
  if (_fd < 0) {
    return -1;
  }

  int rc = ::write(_fd, buf, len);

  // FIXME: we might get a short write!
  if (rc <= 0) {
    close();
  }

  return rc;
}


int Socket::read(char *buf, int len) {
  if (_fd < 0) {
    return -1;
  }

  int rc = ::read(_fd, buf, len - 1);

  if (rc <= 0) {
    close();
  } else {
    buf[rc] = 0; 
  }

  return rc;
}


// vim: ts=2 sw=2 et
