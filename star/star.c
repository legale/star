/* @(#)star.c	1.49 97/04/27 Copyright 1985, 1995 J. Schilling */
#ifndef lint
static	char sccsid[] =
	"@(#)star.c	1.49 97/04/27 Copyright 1985, 1995 J. Schilling";
#endif
/*
 *	Copyright (c) 1985, 1995 J. Schilling
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <mconfig.h>
#include <stdio.h>
#include <signal.h>
#include <stdxlib.h>
#include <unixstd.h>
#include <strdefs.h>
#include "star.h"
#include "diff.h"
#include <waitdefs.h>
#include <standard.h>
#include <device.h>
#include "starsubs.h"

EXPORT	int	main		__PR((int ac, char** av));
LOCAL	void	openlist	__PR((void));
LOCAL	void	usage		__PR((int ret));
LOCAL	void	xusage		__PR((int ret));
LOCAL	void	dusage		__PR((int ret));
LOCAL	void	husage		__PR((int ret));
LOCAL	void	gargs		__PR((int ac, char *const* av));
LOCAL	long	number		__PR((char* arg, int* retp));
LOCAL	int	getnum		__PR((char* arg, long* valp));
EXPORT	BOOL	match		__PR((const char* name));
LOCAL	int	addpattern	__PR((const char* pattern));
LOCAL	int	add_diffopt	__PR((char* optstr, long* flagp));
LOCAL	int	gethdr		__PR((char* optstr, long* typep));
LOCAL	void	exsig		__PR((int sig));
LOCAL	void	sighup		__PR((int sig));
LOCAL	void	sigintr		__PR((int sig));
LOCAL	void	sigquit		__PR((int sig));
LOCAL	void	getstamp	__PR((void));

#if	defined(SIGDEFER) || defined(SVR4)
#define	signal	sigset
#endif

#define	QIC_24_TSIZE	122880		/*  61440 kBytes */
#define	QIC_120_TSIZE	256000		/* 128000 kBytes */
#define	QIC_150_TSIZE	307200		/* 153600 kBytes */
#define	QIC_250_TSIZE	512000		/* 256000 kBytes (XXX not verified)*/
#define	TSIZE(s)	((s)*TBLOCK)

#define	SECOND		(1)
#define	MINUTE		(60 * SECOND)
#define	HOUR		(60 * MINUTE)
#define DAY		(24 * HOUR)
#define YEAR		(365 * DAY)
#define LEAPYEAR	(366 * DAY)

#define	NPAT	10
#define	MAXPAT	100

	int		npat	= 0;
	int		aux[NPAT][MAXPAT];
	int		alt[NPAT];
const	unsigned char	*pat[NPAT];

FILE	*tarf;
FILE	*listf;
FILE	*tty;
char	*tarfile;
char	*listfile;
char	*stampfile;
char	*currdir;
char	*dir_flags = NULL;
char	*volhdr;
#ifdef	FIFO
BOOL	use_fifo = TRUE;
#else
BOOL	use_fifo = FALSE;
#endif
BOOL	shmflag	= FALSE;
long	fs;
long	bs;
int	nblocks = 20;
int	uid;
Ulong	curfs;
long	hdrtype	= H_XSTAR;	/* default header format */
long	chdrtype= H_UNDEF;
int	version	= 0;
int	swapflg	= -1;
BOOL	debug	= FALSE;
BOOL	showtime= FALSE;
BOOL	no_stats= FALSE;
BOOL	do_fifostats= FALSE;
BOOL	numeric	= FALSE;
BOOL	verbose = FALSE;
BOOL	tpath	= FALSE;
BOOL	cflag	= FALSE;
BOOL	uflag	= FALSE;
BOOL	rflag	= FALSE;
BOOL	xflag	= FALSE;
BOOL	tflag	= FALSE;
BOOL	nflag	= FALSE;
BOOL	diff_flag = FALSE;
BOOL	multblk	= FALSE;
BOOL	ignoreerr = FALSE;
BOOL	nodir	= FALSE;
BOOL	nomtime	= FALSE;
BOOL	nochown	= FALSE;
BOOL	acctime	= FALSE;
BOOL	dirmode	= FALSE;
BOOL	nolinkerr = FALSE;
BOOL	follow	= FALSE;
BOOL	nodesc	= FALSE;
BOOL	nomount	= FALSE;
BOOL	interactive = FALSE;
BOOL	signedcksum = FALSE;
BOOL	partial	= FALSE;
BOOL	nospec	= FALSE;
BOOL	uncond	= FALSE;
BOOL	keep_old= FALSE;
BOOL	abs_path= FALSE;
BOOL	notpat	= FALSE;
BOOL	force_hole = FALSE;
BOOL	sparse	= FALSE;
BOOL	to_stdout = FALSE;
BOOL	wready = FALSE;

Ulong	maxsize	= 0L;
Ulong	Newer	= 0L;
Ulong	tsize	= 0L;
BOOL	nowarn	= FALSE;
BOOL	Ctime	= FALSE;

BOOL	listnew	= FALSE;
BOOL	listnewf= FALSE;

int	intr	= 0;

char	*opts = "C*,help,h,xhelp,debug,time,no_statistics,fifostats,numeric,v,tpath,c,u,r,x,t,n,diff,diffopts&,H&,force_hole,sparse,to_stdout,wready,fifo,no_fifo,shm,fs&,VOLHDR*,list*,file*,f*,T,bs&,blocks#,b#,B,pattern&,pat&,i,d,m,nochown,a,atime,p,l,L,D,M,I,O,signed_checksum,P,S,U,k,keep_old_files,/,not,V,maxsize#L,newer*,ctime,tsize#L,qic24,qic120,qic150,qic250,nowarn,newest_file,newest";

EXPORT int
main(ac, av)
	int	ac;
	char	**av;
{
	int		cac  = ac;
	char *const	*cav = av;

	save_args(ac, av);

	--cac,cav++;

	gargs(ac, av);

	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		(void) signal(SIGHUP, sighup);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT, sigintr);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGQUIT, sigquit);

	initbuf(nblocks);

	(void)openremote();		/* This needs super user privilleges */
#ifdef	HAVE_SETREUID
	if (setreuid(-1, getuid()) < 0)
#else
#ifdef	HAVE_SETEUID
	if (seteuid(getuid()) < 0)
#else
	if (setuid(getuid()) < 0)
#endif
#endif
		comerr("Panic cannot set back efective uid.\n");

	opentape();  

	uid = geteuid();
  
	if (stampfile)
		getstamp();

	setprops(chdrtype);	/* Set up properties for archive format */
	dev_init(debug);	/* Init device macro handling */

#ifdef	FIFO
	if (use_fifo)
		runfifo();
#endif

	if (xflag) {
		if (listfile) {
			openlist();
			hash_build(listf, 1000);
		} else {
			for (;getfiles(&cac, &cav, opts);--cac,cav++)
				addpattern(cav[0]);
		}
		extract();
	}
	if (uflag || rflag) {
		skipall();
		syncbuf();
	}
	if (cflag) {
		put_volhdr(volhdr);
		if (listfile) {
			openlist();
			createlist();
		} else {
			if (dir_flags) {
				/*
				 * Skip all other flags.
				 */
				getfiles(&cac, &cav, &opts[3]);
				errmsgno(BAD, "Flag/File: '%s'.\n", cav[0]);
			}
			for (;;--cac,cav++) {
				if (dir_flags)
				if (getargs(&cac, &cav, "C*", &currdir) < 0) {
					errmsgno(BAD,
						"Badly placed Option: %s.\n",
								cav[0]);
					usage(BAD);
				}
				if (currdir) {
					if (chdir(currdir) < 0)
						errmsg("Cannot change directory to '%s'.\n", currdir);
					currdir = NULL;
				}
				if (getfiles(&cac, &cav, opts) == 0)
					break;
				if (intr)
					break;
				curfs = -1L;
				create(cav[0]);
			}
		}
		weof();
		buf_drain();
	}
	if (tflag) {
		if (listfile) {
			openlist();
			hash_build(listf, 1000);
		} else {
			for (;getfiles(&cac, &cav, opts);--cac,cav++)
				addpattern(cav[0]);
		}
		list();
	}
	if (diff_flag) {
		if (listfile) {
			openlist();
			hash_build(listf, 1000);
		} else {
			for (;getfiles(&cac, &cav, opts);--cac,cav++)
				addpattern(cav[0]);
		}
		diff();
	}

	if (!nolinkerr)
		checklinks();
wait(0);
	prstats();
	exit(0);
	/* NOTREACHED */
	return(0);	/* keep lint happy */
}

LOCAL void
openlist()
{
	if (streql(listfile, "-")) {
		listf = stdin;
		listfile = "stdin";
	} else if ((listf = fileopen(listfile, "r")) == (FILE *)NULL)
		comerr("Cannot open '%s'.\n", listfile);
}

LOCAL void
usage(ret)
	int	ret;
{
	error("Usage:\tstar cmd [options] file1 ... filen\n");
	error("Cmd:\n");
	error("\t-c/-u/-r\tcreate/update/replace named files to tape\n");
	error("\t-x/-t/-n\textract/list/trace named files from tape\n");
	error("\t-diff\t\tdiff archive against file system (see -xhelp)\n");
	error("Options:\n");
	error("\t-help,-h\tprint this help\n");
	error("\t-xhelp\t\tprint extended help\n");
	error("\tblocks=#,b=#\tset blocking factor to #x512 Bytes (default 20)\n"); 
	error("\tfile=nm,f=nm\tuse 'nm' as tape instead of stdin/stdout\n");
	error("\t-T\t\tuse $TAPE as tape instead of stdin/stdout\n");
#ifdef	FIFO
	error("\t-fifo/-no_fifo\tuse/don't use a fifo to optimize data flow from/to tape\n");
#if defined(USE_MMAP) && defined(USE_SHM)
	error("\t-shm\t\tuse SysV shared memory for fifo\n");
#endif
#endif
	error("\t-v\t\tbe verbose\n");
	error("\t-tpath\t\tuse with -t to list path names only\n");
	error("\tH=header\tgenerate 'header' type archive (see H=help)\n");
	error("\tC=dir\t\tperform a chdir to 'dir' before storing next file\n");
	error("\t-B\t\tperform multiple reads (needed on pipes)\n");
	error("\t-i\t\tignore checksum errors\n");
	error("\t-d\t\tdo not store/create directories\n");
	error("\t-m\t\tdo not restore access and modification time\n");
	error("\t-nochown\tdo not restore owner and group\n");
	error("\t-a,-atime\treset access time after storing file\n");
	error("\t-p\t\trestore filemodes of directories\n");
	error("\t-l\t\tdo not print a message if not all links are dumped\n");
	error("\t-L\t\tfollow symbolic links as if they were files\n");
	error("\t-D\t\tdo not descend directories\n");
	error("\t-M\t\tdo not descend mounting points\n");
	error("\t-I\t\tdo interactive renaming\n");
	error("\t-O\t\tbe compatible to old tar (except for checksum bug)\n");
	error("\t-P\t\tlast record may be partial (useful on cartridge tapes)\n");
	error("\t-S\t\tdo not store/create special files\n");
	error("\t-U\t\trestore files unconditionally\n");
	exit(ret);
	/* NOTREACHED */
}

LOCAL void
xusage(ret)
	int	ret;
{
	error("Usage:\tstar cmd [options] file1 ... filen\n");
	error("Extended options:\n");
	error("\tdiffopts=optlst\tcomma separated list of diffopts (see diffopts=help)\n");
	error("\t-not,-V\t\tuse those files which do not match pattern\n");
	error("\tVOLHDR=name\tuse name to generate a volume header\n");
	error("\t-keep_old_files,-k\tkeep existing files\n");
	error("\t-/\t\tdon't strip leading '/'s from file names\n");
	error("\tlist=name\tread filenames from named file\n");
	error("\tpattern=p,pat=p\tset matching pattern\n");
	error("\tmaxsize=#\tdo not store file if it is bigger than # kBytes\n");
	error("\tnewer=name\tstore only files which are newer than 'name'\n");
	error("\t-ctime\t\tuse ctime for newer= option\n");
	error("\tbs=#\t\tset (output) block size to #\n");
#ifdef	FIFO
	error("\tfs=#\t\tset fifo size to #\n");
#endif
	error("\ttsize=#\t\tset tape volume size to # 512 byte blocks\n");
	error("\t-qic24\t\tset tape volume size to %d kBytes\n",
						TSIZE(QIC_24_TSIZE)/1024);
	error("\t-qic120\t\tset tape volume size to %d kBytes\n",
						TSIZE(QIC_120_TSIZE)/1024);
	error("\t-qic150\t\tset tape volume size to %d kBytes\n",
						TSIZE(QIC_150_TSIZE)/1024);
	error("\t-qic250\t\tset tape volume size to %d kBytes\n",
						TSIZE(QIC_250_TSIZE)/1024);
	error("\t-nowarn\t\tdo not print warning messages\n");
	error("\t-time\t\tprint timing info\n");
	error("\t-no_statistics\tdo not print statistics\n");
	error("\t-fifostats\tprint fifo statistics\n");
	error("\t-numeric\tdon't use user/group name from tape\n");
	error("\t-newest\t\tfind newest file on tape\n");
	error("\t-newest_file\tfind newest regular file on tape\n");
	error("\t-signed_checksum\tuse signed chars to calculate checksum\n");
	error("\t-sparse\t\thandle file with holes effectively on store/create\n");
	error("\t-force_hole\ttry to extract all files with holes\n");
	error("\t-to_stdout\textract files to stdout\n");
	error("\t-wready\t\twait for tape drive to become ready\n");
	exit(ret);
	/* NOTREACHED */
}

LOCAL void
dusage(ret)
	int	ret;
{
	error("Diff options:\n");
	error("\tnot\t\tif this option is present, exclude listed options\n");
	error("\tperm\t\tcompare file permissions\n");
	error("\tmode\t\tcompare file permissions\n");
	error("\ttype\t\tcompare file type\n");
	error("\tnlink\t\tcompare linkcount (not supported)\n");
	error("\tuid\t\tcompare owner of file\n");
	error("\tgid\t\tcompare group of file\n");
	error("\tuname\t\tcompare name of owner of file\n");
	error("\tgname\t\tcompare name of group of file\n");
	error("\tid\t\tcompare owner, group, ownername and groupname of file\n");
	error("\tsize\t\tcompare file size\n");
	error("\tdata\t\tcompare content of file\n");
	error("\tcont\t\tcompare content of file\n");
	error("\trdev\t\tcompare rdev of device node\n");
	error("\thardlink\tcompare target of hardlink\n");
	error("\tsymlink\t\tcompare target of symlink\n");
	error("\tatime\t\tcompare access time of file (only star)\n");
	error("\tmtime\t\tcompare modification time of file\n");
	error("\tctime\t\tcompare creation time of file (only star)\n");
	error("\ttimes\t\tcompare all times of file\n");
	exit(ret);
	/* NOTREACHED */
}

LOCAL void
husage(ret)
	int	ret;
{
	error("Header types:\n");
	error("\ttar\t\told tar format\n");
	error("\tstar\t\tstar format\n");
	error("\tgnutar\t\tgnu tar format\n");
	error("\tustar\t\tstandard tar (ieee 1003.1) format\n");
	error("\txstar\t\textended standard tar format\n");
	exit(ret);
	/* NOTREACHED */
}

LOCAL void
gargs(ac, av)
	int		ac;
	char	*const *av;
{
	BOOL	help	= FALSE;
	BOOL	xhelp	= FALSE;
	BOOL	oldtar	= FALSE;
	BOOL	no_fifo	= FALSE;
	BOOL	usetape	= FALSE;
	BOOL	qic24	= FALSE;
	BOOL	qic120	= FALSE;
	BOOL	qic150	= FALSE;
	BOOL	qic250	= FALSE;

/*char	*opts = "C*,help,h,xhelp,debug,time,no_statistics,fifostats,numeric,v,tpath,c,u,r,x,t,n,diff,diffopts&,H&,force_hole,sparse,to_stdout,wready,fifo,no_fifo,shm,fs&,VOLHDR*,list*,file*,f*,T,bs&,blocks#,b#,B,pattern&,pat&,i,d,m,nochown,a,atime,p,l,L,D,M,I,O,signed_checksum,P,S,U,k,keep_old_files,/,not,V,maxsize#L,newer*,ctime,tsize#L,qic24,qic120,qic150,qic250,nowarn,newest_file,newest";*/

	--ac,++av;  
	if (getallargs(&ac, &av, opts,
				&dir_flags,
				&help, &help, &xhelp, &debug,
				&showtime, &no_stats, &do_fifostats,
				&numeric, &verbose, &tpath,
#ifndef	lint
				&cflag,
				&uflag,
				&rflag,
				&xflag,
				&tflag,
				&nflag,
				&diff_flag, add_diffopt, &diffopts,
				gethdr, &chdrtype,
				&force_hole, &sparse, &to_stdout, &wready,
				&use_fifo, &no_fifo, &shmflag,
				getnum, &fs,
				&volhdr,
				&listfile,
				&tarfile, &tarfile,
				&usetape,
				getnum, &bs,
				&nblocks, &nblocks,
				&multblk,
				addpattern, NULL,
				addpattern, NULL,
				&ignoreerr,
				&nodir,
				&nomtime, &nochown,
				&acctime, &acctime,
				&dirmode,
				&nolinkerr,
				&follow,
				&nodesc,
				&nomount,
				&interactive,
				&oldtar, &signedcksum,
				&partial,
				&nospec,
				&uncond,
				&keep_old, &keep_old,
				&abs_path,
				&notpat, &notpat,
				&maxsize,
				&stampfile,
				&Ctime,
				&tsize,
				&qic24,
				&qic120,
				&qic150,
				&qic250,
				&nowarn,
#endif /* lint */
				&listnewf,
				&listnew) < 0){
		errmsgno(BAD, "Bad Option: %s.\n", av[0]);
		usage(BAD);
	}
	if (help)
		usage(0);
	if (xhelp)
		xusage(0);

	if ((xflag + cflag + uflag + rflag + tflag + nflag + diff_flag) > 1) {
		errmsgno(BAD, "Only one of -x -c -u -r -t or -n.\n");
		usage(BAD);
	}
	if (!(xflag | cflag | uflag | rflag | tflag | nflag | diff_flag)) {
		errmsgno(BAD, "Must specify -x -c -u -r -t -n -diff.\n");
		usage(BAD);
	}
	if (uflag || rflag) {
		errmsgno(BAD, "-u and -r currently not implemented sorry.\n");
		if (uflag)
			usage(BAD);
		cflag = TRUE;
	}
	if (no_fifo)
		use_fifo = FALSE;
#ifndef	FIFO
	if (use_fifo) {
		errmsgno(BAD, "Fifo not configured in.\n");
		usage(BAD);
	}
#endif
	if (oldtar)
		chdrtype = H_OTAR;
	if (chdrtype != H_UNDEF) {
		if (H_TYPE(chdrtype) == H_OTAR)
			oldtar = TRUE;	/* XXX hack */
	}
	if (cflag) {
		if (chdrtype != H_UNDEF)
			hdrtype = chdrtype;
		chdrtype = hdrtype;	/* wegen setprops in main() */

		/*
		 * hdrtype und chdrtype
		 * bei uflag, rflag sowie xflag, tflag, nflag, diff_flag
		 * in get_tcb vergleichen !
		 */
	}
	if (diff_flag) {
		if (diffopts == 0)
			diffopts = D_DEFLT;
	} else if (diffopts != 0) {
		errmsgno(BAD, "diffopts= only makes sense with -diff\n");
		usage(BAD);
	}
	if (bs % TBLOCK) {
		errmsgno(BAD, "Invalid blocksize %ld.\n", bs);
		usage(BAD);
	}
	if (bs)
		nblocks = bs / TBLOCK;
	if (nblocks <= 0) {
		errmsgno(BAD, "Invalid blocksize %d blocks.\n", nblocks);
		usage(BAD);
	}
	bs = nblocks * TBLOCK;
	if (tsize > 0 && tsize < 3) {
		errmsgno(BAD, "Tape size must be at least 3 blocks.\n");
		usage(BAD);
	}
	if (tsize == 0) {
		if (qic24)  tsize = QIC_24_TSIZE;
		if (qic120) tsize = QIC_120_TSIZE;
		if (qic150) tsize = QIC_150_TSIZE;
		if (qic250) tsize = QIC_250_TSIZE;
	}
	if (listfile != NULL)
		nodesc = TRUE;
	if (oldtar)
		nospec = TRUE;
	if (!tarfile) {
		if (usetape) {
			tarfile = getenv("TAPE");
		}
		if (!tarfile)
			tarfile = "-";
	}
	if (interactive || tsize > 0) {
#ifdef	JOS
		tty = stderr;
#else
		if ((tty = fileopen("/dev/tty", "r")) == (FILE *)NULL)
			comerr("Cannot open '/dev/tty'.\n");
#endif
	}
	if (nflag) {
		xflag = TRUE;
		interactive = TRUE;
		verbose = TRUE;
	}
	if (to_stdout) {
		force_hole = FALSE;
	}
}

LOCAL long
number(arg, retp)
	register char	*arg;
		int	*retp;
{
	long	val	= 0;

	if (*retp != 1)
		return (val);
	if (*arg == '\0')
		*retp = -1;
	else if (*(arg = astol(arg, &val))) {
		if (*arg == 'm' || *arg == 'M') {
			val *= (1024*1024);
			arg++;
		}
		else if (*arg == 'k' || *arg == 'K') {
			val *= 1024;
			arg++;
		}
		else if (*arg == 'b' || *arg == 'B') {
			val *= TBLOCK;
			arg++;
		}
		else if (*arg == 'w' || *arg == 'W') {
			val *= 2;
			arg++;
		}
		if (*arg == '*' || *arg == 'x')
			val *= number(++arg, retp);
		else if (*arg != '\0')
			*retp = -1;
	}
	return (val);
}

LOCAL int
getnum(arg, valp)
	char	*arg;
	long	*valp;
{
	int	ret = 1;

	*valp = number(arg, &ret);
	return (ret);
}

EXPORT BOOL
match(name)
	const	char	*name;
{
	register int	i;
		char	*ret = NULL;

	for (i=0; i < npat; i++) {
		ret = (char *)patmatch(pat[i], aux[i],
					(const unsigned char *)name, 0,
					strlen(name), alt[i]);
		if (ret != NULL && *ret == '\0')
			break;
	}
	if (notpat ^ (ret != NULL && *ret == '\0'))
		return TRUE;
	return FALSE;
}

LOCAL int
addpattern(pattern)
	const char	*pattern;
{
	int	plen;

	if (npat >= NPAT)
		comerrno(BAD, "Too many patterns.\n");
	plen = strlen(pattern);
	pat[npat] = (const unsigned char *)pattern;
	if (plen > MAXPAT)
		comerrno(BAD, "Pattern too long: %d, max is %d.\n",
			plen, MAXPAT);
	if ((alt[npat] = patcompile((const unsigned char *)pattern,
							plen, aux[npat])) == 0)
		comerrno(BAD, "Bad pattern: '%s'.\n", pattern);
	npat++;
	return (TRUE);
}

LOCAL int
add_diffopt(optstr, flagp)
	char	*optstr;
	long	*flagp;
{
	char	*ep;
	char	*np;
	int	optlen;
	long	optflags = 0;
	BOOL	not = FALSE;

	while (*optstr) {
		if ((ep = strchr(optstr, ',')) != NULL) {
			optlen = ep - optstr;
			np = &ep[1];
		} else {
			optlen = strlen(optstr);
			np = &optstr[optlen];
		}
		if (optstr[0] == '!') {
			optstr++;
			optlen--;
			not = TRUE;
		}
		if (strncmp(optstr, "not", optlen) == 0 ||
				strncmp(optstr, "!", optlen) == 0) {
			not = TRUE;
		} else if (strncmp(optstr, "all", optlen) == 0) {
			optflags |= D_ALL;
		} else if (strncmp(optstr, "perm", optlen) == 0) {
			optflags |= D_PERM;
		} else if (strncmp(optstr, "mode", optlen) == 0) {
			optflags |= D_PERM;
		} else if (strncmp(optstr, "type", optlen) == 0) {
			optflags |= D_TYPE;
		} else if (strncmp(optstr, "nlink", optlen) == 0) {
			optflags |= D_NLINK;
			errmsgno(BAD, "nlink not supported\n");
			dusage(BAD);
		} else if (strncmp(optstr, "uid", optlen) == 0) {
			optflags |= D_UID;
		} else if (strncmp(optstr, "gid", optlen) == 0) {
			optflags |= D_GID;
		} else if (strncmp(optstr, "uname", optlen) == 0) {
			optflags |= D_UNAME;
		} else if (strncmp(optstr, "gname", optlen) == 0) {
			optflags |= D_GNAME;
		} else if (strncmp(optstr, "id", optlen) == 0) {
			optflags |= D_ID;
		} else if (strncmp(optstr, "size", optlen) == 0) {
			optflags |= D_SIZE;
		} else if (strncmp(optstr, "data", optlen) == 0) {
			optflags |= D_DATA;
		} else if (strncmp(optstr, "cont", optlen) == 0) {
			optflags |= D_DATA;
		} else if (strncmp(optstr, "rdev", optlen) == 0) {
			optflags |= D_RDEV;
		} else if (strncmp(optstr, "hardlink", optlen) == 0) {
			optflags |= D_HLINK;
		} else if (strncmp(optstr, "symlink", optlen) == 0) {
			optflags |= D_SLINK;
		} else if (strncmp(optstr, "atime", optlen) == 0) {
			optflags |= D_ATIME;
		} else if (strncmp(optstr, "mtime", optlen) == 0) {
			optflags |= D_MTIME;
		} else if (strncmp(optstr, "ctime", optlen) == 0) {
			optflags |= D_CTIME;
		} else if (strncmp(optstr, "times", optlen) == 0) {
			optflags |= D_TIMES;
		} else if (strncmp(optstr, "help", optlen) == 0) {
			dusage(0);
		} else {
			error("Illegal diffopt.\n");
			dusage(BAD);
			return (-1);
		}
		optstr = np;
	}
	if (not) {
		*flagp = ~optflags;
	} else {
		*flagp = optflags;
	}
	return (TRUE);
}

LOCAL int
gethdr(optstr, typep)
	char	*optstr;
	long	*typep;
{
	BOOL	swapped = FALSE;
	long	type	= H_UNDEF;

	if (*optstr == 'S') {
		swapped = TRUE;
		optstr++;
	}
	if (streql(optstr, "tar")) {
		type = H_OTAR;
	} else if (streql(optstr, "star")) {
		type = H_STAR;
	} else if (streql(optstr, "gnutar")) {
		type = H_GNUTAR;
	} else if (streql(optstr, "ustar")) {
		type = H_USTAR;
	} else if (streql(optstr, "xstar")) {
		type = H_XSTAR;
	} else if (streql(optstr, "help")) {
		husage(0);
	} else {
		error("Illegal header type '%s'.\n", optstr);
		husage(BAD);
		return (-1);
	}
	if (swapped)
		*typep = H_SWAPPED(type);
	else
		*typep = type;
	return (TRUE);
}

LOCAL void
exsig(sig)
	int	sig;
{
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
}

/* ARGSUSED */
LOCAL void
sighup(sig)
	int	sig;
{
	signal(SIGHUP, sighup);
	prstats();
	intr++;
	if (!cflag)
		exsig(sig);
}

/* ARGSUSED */
LOCAL void
sigintr(sig)
	int	sig;
{
	signal(SIGINT, sigintr);
	prstats();
	intr++;
	if (!cflag)
		exsig(sig);
}

/* ARGSUSED */
LOCAL void
sigquit(sig)
	int	sig;
{
	signal(SIGQUIT, sigquit);
	prstats();
}

LOCAL void
getstamp()
{
	FINFO	finfo;
	BOOL	ofollow = follow;

	follow = TRUE;
	if (!getinfo(stampfile, &finfo))
		comerr("Cannot stat '%s'.\n", stampfile);
	follow = ofollow;

	Newer = finfo.f_mtime;
}
