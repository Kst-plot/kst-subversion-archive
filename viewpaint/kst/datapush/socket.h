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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _SOCKET_H
#define _SOCKET_H

struct sockaddr_in;

class Socket {
  public:
    Socket();
    virtual ~Socket();

    virtual int open(const char *addr, int port);

    virtual void close();

    bool connected() const { return _fd >= 0; }

    int descriptor() const { return _fd; }

    int read(char *buf, int len);
    int write(const char *buf, int len);

  private:
    static void init();
    static bool _init;

  protected:
    int resolv(const char *addr, int port);
    struct sockaddr_in *_client;
    int _fd;

    // Why is this friend needed?
    friend class Listener;
    Socket(struct sockaddr_in *peer, int fd);
};

#endif

// vim: ts=2 sw=2 et
