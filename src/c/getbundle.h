/**
 * File              : getbundle.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 07.10.2022
 * Last Modified Date: 21.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/**
 * getbundle.h
 * Copyright (c) 2022 Igor V. Sementsov <ig.kuzm@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Get bundle of application
 * for mac os - APP/Contents/Resources,
 * for win - executable directory
 * for linux/unix - /usr/[local]/share/APP
 */

#ifndef k_lib_getbundle_h__
#define k_lib_getbundle_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> //dirname
#if defined __APPLE__
#elif defined _WIN32
#include <Windows.h>
#else
#include <unistd.h> //readlink
#endif

/*
 * getbundle
 * Get bundle of application
 * for mac os - APP/Contents/Resources,
 * for win - executable directory
 * for linux/unix - /usr/[local]/share/APP
 * %argv - arguments of main() function
 * */
static char *getbundle(char *argv[]) {
  if (!argv || !argv[0])
    return NULL;
#ifdef _WIN32
  return dirname((char *)argv[0]);
#else
  char *bundle = (char *)malloc(BUFSIZ);
  if (!bundle)
    return NULL;
#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_MAC
  sprintf(bundle, "%s%s", dirname((char *)argv[0]), "/../Resources");
  return bundle;
#else
  free(bundle);
  return dirname((char *)argv[0]);
#endif
#else
  char selfpath[128];
  if (readlink("/proc/self/exe", selfpath, sizeof(selfpath) - 1) < 0) {
    free(bundle);
    return NULL;
  }
  sprintf(bundle, "%s/../share/%s", dirname(selfpath), basename(argv[0]));
  return bundle;
#endif
#endif
  return NULL;
}

#ifdef __cplusplus
}
#endif
#endif // k_lib_getbundle_h__
