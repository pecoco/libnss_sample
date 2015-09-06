// Ubuntu 14.04
// gcc -Wall -g -O2 -shared  -fPIC -DPIC -o libnss_sample.so libnss_sample.cpp
// ln -s `pwd`/libnss_sample.so /lib/x86_64-linux-gnu/libnss_sample.so.2

// vi /etc/nsswitch.conf
/* -------------------------------------------------------------------------------
# /etc/nsswitch.conf
#
# Example configuration of GNU Name Service Switch functionality.
# If you have the `glibc-doc-reference' and `info' packages installed, try:
# `info libc "Name Service Switch"' for information about this file.

passwd:         compat sample
group:          compat sample
shadow:         compat sample

hosts:          files mdns4_minimal [NOTFOUND=return] dns
networks:       files

protocols:      db files
services:       db files
ethers:         db files
rpc:            db files

netgroup:       nis
------------------------------------------------------------------------------- */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdio_ext.h>
#include <string.h>
#include <nss.h>
#include <syslog.h>
#include <stack>

#define logput(type, msg) { \
  openlog("nsswitch", LOG_CONS | LOG_PID, LOG_USER); \
  syslog(type, msg); \
  closelog(); \
}
 
#define FIXED_HOME  "/tmp"
#define FIXED_SHELL "/bin/bash"
#define FIXED_UID   10000
#define FIXED_GID   10000

#define FIXED_SAMPLE  "sample"

extern "C"
int
_nss_sample_getpwnam_r (const char *name, struct passwd *pwd, char *buffer,
                       size_t buflen, struct passwd **result)
{
  logput(LOG_NOTICE, __FUNCTION__);
  //struct passwd {
  //  char   *pw_name;       /* ユーザー名 */
  //  char   *pw_passwd;     /* ユーザーのパスワード */
  //  uid_t   pw_uid;        /* ユーザー ID */
  //  gid_t   pw_gid;        /* グループ ID */
  //  char   *pw_gecos;      /* ユーザー情報 */
  //  char   *pw_dir;        /* ホームディレクトリ */
  //  char   *pw_shell;      /* シェルプログラム */
  //};

  size_t len = 0;
  len += strlen(name) + 1;
  len += strlen(FIXED_HOME) + 1;
  len += strlen(FIXED_SHELL) + 1;
  len += 2; // ""

  if (buflen < len) {
    if (result != NULL) result = NULL;
    errno = ERANGE;
    logput(LOG_NOTICE, "error");
    return (ERANGE);
  }

  char* p = buffer;
  strcpy(p, name);
  pwd->pw_name = p;
  p += strlen(name) + 1;

  strcpy(p, FIXED_HOME);
  pwd->pw_dir = p;
  p += strlen(FIXED_HOME) + 1;

  strcpy(p, FIXED_SHELL);
  pwd->pw_shell = p;
  p += strlen(FIXED_SHELL) + 1;

  pwd->pw_uid = FIXED_UID;
  pwd->pw_gid = FIXED_GID;

  strcpy(p, "");
  pwd->pw_gecos = p;
  pwd->pw_passwd = p;

  if (result != NULL) *result = pwd;
  logput(LOG_NOTICE, "success");
  return (1);
}

extern "C"
int
_nss_sample_getgrnam_r (const char *name, struct group *grp,
          char *buffer, size_t buflen, struct group **result)
{
  logput(LOG_NOTICE, __FUNCTION__);
  //struct group {
  //    char   *gr_name;        /* グループ名 */
  //    char   *gr_passwd;      /* グループのパスワード */
  //    gid_t   gr_gid;         /* グループ ID */
  //    char  **gr_mem;         /* グループのメンバ名へのポインター
  //                               の配列 (配列はヌルで終端する) */
  //};

  size_t len = 0;
  len += strlen(name) + 1;
  len += strlen(FIXED_SAMPLE) + 1;
  len += 2; // ""

  if (buflen < len) {
    if (result != NULL) result = NULL;
    errno = ERANGE;
    logput(LOG_NOTICE, "error");
    return (ERANGE);
  }

  char* p = buffer;
  strcpy(p, name);
  grp->gr_name = p;
  p += strlen(name) + 1;

  strcpy(p, "");
  grp->gr_passwd = p;

  grp->gr_gid = FIXED_GID;
  grp->gr_mem[0] = NULL;

  if (result != NULL) *result = grp;
  logput(LOG_NOTICE, "success");
  return (1);
}

int _nss_sample_getpwnam_r_test() {
	struct passwd st_pwd;
	struct passwd *pst_res = NULL;
	char buffer[4096];
	int res =  _nss_sample_getpwnam_r ("abc", &st_pwd, buffer, sizeof(buffer), &pst_res);
	printf("%d\n", res);
	if (res != 0) {
		printf("%d\n", st_pwd.pw_uid);
		printf("%d\n", st_pwd.pw_gid);
		printf("%s\n", st_pwd.pw_name);
		printf("%s\n", st_pwd.pw_dir);
		printf("%s\n", st_pwd.pw_shell);
	} else {
		return 1;
	}

	return 0;
}

int getpwnam_test() {
	struct passwd* test = NULL;

	test = getpwnam("root");
	if (test != NULL) {
		printf("%d\n",   test->pw_uid);
		printf("%d\n",   test->pw_gid);
		printf("%s\n",   test->pw_name);
		printf("%s\n",   test->pw_dir);
		printf("%s\n\n", test->pw_shell);
	} else {
		return 1;
	}

	test = getpwnam("abcdefghijklmnopqrstuvwxyz");
	if (test != NULL) {
		printf("%d\n",   test->pw_uid);
		printf("%d\n",   test->pw_gid);
		printf("%s\n",   test->pw_name);
		printf("%s\n",   test->pw_dir);
		printf("%s\n\n", test->pw_shell);
	} else {
		return 1;
	}

	return 0;
}

int getgrnam_test() {
	struct group* test = NULL;

	test = getgrnam("root");
	if (test != NULL) {
		printf("%s\n",   test->gr_name);
		printf("%s\n",   test->gr_passwd);
		printf("%d\n",   test->gr_gid);
		printf("%p\n\n", test->gr_mem);
		printf("%s\n\n", test->gr_mem[0]);
	} else {
		return 1;
	}

	test = getgrnam("abcdefghijklmnopqrstuvwxyz");
	if (test != NULL) {
		printf("%s\n",   test->gr_name);
		printf("%s\n",   test->gr_passwd);
		printf("%d\n",   test->gr_gid);
		printf("%p\n\n", test->gr_mem);
		printf("%s\n\n", test->gr_mem[0]);
	} else {
		return 1;
	}

	return 0;
}

int main() {
	//_nss_sample_getpwnam_r_test();
	getpwnam_test();
	getgrnam_test();
	return 0;
}

