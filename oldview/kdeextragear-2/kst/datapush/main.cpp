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


#include <iostream>
#include <list>
#include <queue>
#include <vector>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "listener.h"

using namespace std;


int parseFormatFile(FILE *fp, std::vector<std::string>& filenames);
int mainLoop(Listener *l, FILE *fp);
int handlePassiveMode(int& n, std::list<Socket*>& sockets, fd_set& rfds, fd_set& efds);
int handleCommandMode(int& n, std::list<Socket*>& sockets, std::list<Socket*>& activationList, fd_set& rfds, fd_set& efds, int fd);

void usage(const char *me) {
  cerr << "usage: " << me << " [options] [format file]" << endl;
  exit(-1);
}


static struct option long_options[] = {
  { "help", 0, 0, 'h' },
  { "port", 1, 0, 'p' },
  { 0, 0, 0, 0 }
};

static int port = 4242;
static const char *filename = 0L;
static bool foreground = false;

int main(int argc, char **argv) {
  int idx = 0;
  const char *me = argv[0];

  // parse options
  for (;;) {
    int c = getopt_long(argc, argv, "fp:h?", long_options, &idx);
    if (c == -1) {
      break;
    }

    switch (c) {
    case 'f':
        foreground = true;
      break;
    case 'p':
        port = atoi(optarg);
      break;
    case 'h':
    case '?':
      usage(me);
    default:
      break;
    }
  }

  if (optind != argc - 1) {
    usage(me);
  }

  filename = argv[optind++];

  // open the file
  FILE *fp = fopen(filename, "r+");
  if (!fp) {
    perror(filename);
    return -1;
  }

  // try to parse it
  std::vector<std::string> filenames;
  if (0 != parseFormatFile(fp, filenames)) {
    fclose(fp);
    return -1;
  }

  // open the listener socket
  Listener *l = new Listener(port);
  if (!l->listening()) {
    perror("listen");
    cerr << "Error opening port " << port << "." << endl;
    return -1;
  }

  if (!foreground) {
    // daemonize
    int pid = 0;
    if ((pid = fork()) != 0) {
      cout << "Backgrounding with process id " << pid << "." << endl;
      cout << "Listening on port " << port << "." << endl;
    } else {
      return mainLoop(l, fp);
    }
  } else {
    return mainLoop(l, fp);
  }

return 0;
}


static void clearList(std::list<Socket*> &s) {
    for (std::list<Socket*>::iterator it = s.begin(); it != s.end(); ++it) {
      delete *it;
    }
    s.clear();
}


int mainLoop(Listener *l, FILE *fp) {
  std::list<Socket*> sockets, idle;
  int fd = fileno(fp);
#define MAX(x,y) ((x) > (y) ? (x) : (y))
  int n = l->descriptor() + 1;
#undef MAX
  fd_set rfds, efds;
  long fsize = 0;
  struct stat sbuf;
  struct timeval tv;

  if (fstat(fd, &sbuf) < 0) {
    return -1;
  }

  fsize = sbuf.st_size;

  for (;;) {
    FD_ZERO(&rfds);
    FD_ZERO(&efds);

    FD_SET(l->descriptor(), &rfds);
    for (std::list<Socket*>::iterator it = idle.begin(); it != idle.end(); ++it) {
      FD_SET((*it)->descriptor(), &rfds);
      FD_SET((*it)->descriptor(), &efds);
    }

    for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
      FD_SET((*it)->descriptor(), &rfds);
      FD_SET((*it)->descriptor(), &efds);
    }

    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    int rc = select(n, &rfds, 0L, &efds, &tv);

    if (rc == 0) { // timeout
      if (fstat(fd, &sbuf) < 0 || sbuf.st_size < fsize) {
        clearList(sockets);
        clearList(idle);
        return -1;
      }

      if (sbuf.st_size == fsize) {
        continue;
      }

      if (lseek(fd, fsize, SEEK_SET) != fsize) {
        clearList(sockets);
        clearList(idle);
        return -1;
      }

      long readsz = sbuf.st_size - fsize;
      while (readsz > 0) {
        char buf[1024];
#define MIN(x,y) ((x) < (y) ? (x) : (y))
        long len = MIN(readsz, 1024);
#undef MIN
        readsz -= len;
        if (read(fd, buf, len) != len) {
          clearList(sockets);
          clearList(idle);
          return -1;
        }

        for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
          // FIXME: error codes
          (*it)->write(buf, len);
        }
      }
 
      fsize = sbuf.st_size;
      continue;
    }

    if (rc < 0) {
      break;
    } 

    if (rc > 0) {
      n = l->descriptor();

      if (0 > handlePassiveMode(n, sockets, rfds, efds)) {
        return -1;
      }

      if (0 > handleCommandMode(n, idle, sockets, rfds, efds, fileno(fp))) {
        return -1;
      }

      n++;

      // new inbound connection
      if (FD_ISSET(l->descriptor(), &rfds)) {
        Socket *s = l->accept();
        if (s) {
          if (s->descriptor() >= n) {
            n = s->descriptor() + 1;
          }
          idle.push_back(s);
        }
      }
    }
  }

  clearList(idle);
  clearList(sockets);
  return 0;
}


int handleCommandMode(int& n, std::list<Socket*>& sockets, std::list<Socket*>& activationList, fd_set& rfds, fd_set& efds, int fd) {
  std::vector<Socket*> trash;
  std::vector<Socket*> activations;
  for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
    Socket *s = *it;
    int desc = s->descriptor();
    bool trashed = false;

    if (FD_ISSET(desc, &efds)) { // exception occurred
      trashed = true;
      cout << "Exception detected on socket " << desc << endl;
    } else if (FD_ISSET(desc, &rfds)) {
      char buf[1024];
      if (0 < s->read(buf, 1024)) {
        if (0 == ::strncmp(buf, "quit", 4)) {  // user requested quit
          s->close();
        } else if (0 == ::strncmp(buf, "format", 6)) {  // user requested format
          ::lseek(fd, 0, SEEK_SET);
          int rc;
          while ((rc = ::read(fd, buf, 1024)) > 0) {
            s->write(buf, rc);
          }
        } else if (0 == ::strncmp(buf, "go", 2)) {
          activations.push_back(s);
/*
        } else { // a number?  dump starting at that point in the file.
          long pt = 0;
          buf[10] = 0; // truncate
          sscanf(buf, "%ld", &pt);
          if (pt >= 0 && ::lseek(fd, pt, SEEK_SET) == pt) {
            long readsz = fsize - pt;

            sprintf(buf, "%ld\n", readsz >= 0 ? readsz : 0);
            s->write(buf, strlen(buf));

            while (readsz > 0) {
#define MIN(x,y) ((x) < (y) ? (x) : (y))
              long len = MIN(readsz, 1024);
#undef MIN
              readsz -= len;
              if (::read(fd, buf, len) != len) {
                clearList(sockets);
                return -1;
              }
              s->write(buf, len);
            }
          } else {
            s->write("0\n", 2);
          }
*/
        }
      }

      if (!s->connected()) {
        trashed = true;
      }
    }

    if (trashed) {    // add to the trash (deleted sockets)
      trash.push_back(s);
    } else {
      if (desc > n) {
        n = desc;
      }
    }
  }

  while (!trash.empty()) {  // clean up the trash
    sockets.remove(trash.back());
    delete trash.back();
    trash.pop_back();
  }

  while (!activations.empty()) {  // do activations
    sockets.remove(activations.back());
    activationList.push_back(activations.back());
    activations.pop_back();
  }

return 0;
}


int handlePassiveMode(int& n, std::list<Socket*>& sockets, fd_set& rfds, fd_set& efds) {
  std::vector<Socket*> trash;
  for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
    Socket *s = *it;
    int desc = s->descriptor();
    bool trashed = false;

    if (FD_ISSET(desc, &efds)) { // exception occurred
      trashed = true;
      cout << "Exception detected on socket " << desc << endl;
    } else if (FD_ISSET(desc, &rfds)) {
      char buf[1024];
      if (0 < s->read(buf, 1024)) {
        if (0 == ::strncmp(buf, "quit", 4)) {  // user requested quit
          s->close();
        }
      }

      if (!s->connected()) {
        trashed = true;
      }
    }

    if (trashed) {    // add to the trash (deleted sockets)
      trash.push_back(s);
    } else {
      if (desc > n) {
        n = desc;
      }
    }
  }

  while (!trash.empty()) {  // clean up the trash
    sockets.remove(trash.back());
    delete trash.back();
    trash.pop_back();
  }

return 0;
}


int parseFormatFile(FILE *fp, std::vector<std::string>& filenames) {
char buf[256];

  while (!::feof(fp)) {
    if (::fgets(buf, 256, fp)) {
      char *tok = ::strtok(buf, " \n\r\t");
      if (tok && tok[0] != '#') {
        std::string field = tok;
        std::vector<std::string> args;
        while ((tok = ::strtok(0L, " \n\r\t")) != 0L) {
          args.push_back(tok);
        }

        if (!args.empty() && args.front() == "RAW") {
          filenames.push_back(field);
        }
      }
    }
  }

return 0;
}

// vim: ts=2 sw=2 et
