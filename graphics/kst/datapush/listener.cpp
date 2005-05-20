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

#include "listener.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>


Listener::Listener(int port)
: Socket() {
  if (port < 0 || !_client) {
    return;
  }

  ::memset(_client, 0, sizeof(_client));

  _fd = ::socket(PF_INET, SOCK_STREAM, 0);

  if (_fd < 0) {
    return;
  }

  int tmp = 1;
  setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));

  _client->sin_family = AF_INET;
  _client->sin_addr.s_addr = INADDR_ANY;
  _client->sin_port = ntohs(port);

  if (0 > ::bind(_fd, reinterpret_cast<struct sockaddr*>(_client), sizeof(struct sockaddr_in))) {
    close();
    return;
  }

  if (0 > ::listen(_fd, 10)) {
    close();
    return;
  }
}


Listener::~Listener() {
}


Socket *Listener::accept() {
  int sz = sizeof(struct sockaddr_in);
  struct sockaddr *peer = static_cast<struct sockaddr*>(::malloc(sz));

  int fd = ::accept(_fd, peer, reinterpret_cast<socklen_t*>(&sz));
  if (fd >= 0) {
    return new Socket(reinterpret_cast<struct sockaddr_in*>(peer), fd);
  } else {
    ::free(peer);
    return 0L;
  }
}


// vim: ts=2 sw=2 et
